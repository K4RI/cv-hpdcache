/**
 *  Copyright 2023,2024 CEA*
 *  *Commissariat a l'Energie Atomique et aux Energies Alternatives (CEA)
 *  Copyright 2025 Inria, Universite Grenoble-Alpes, TIMA
 *
 *  SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 *  Licensed under the Solderpad Hardware License v 2.1 (the “License”); you
 *  may not use this file except in compliance with the License, or, at your
 *  option, the Apache License version 2.0. You may obtain a copy of the
 *  License at
 *
 *  https://solderpad.org/licenses/SHL-2.1/
 *
 *  Unless required by applicable law or agreed to in writing, any work
 *  distributed under the License is distributed on an “AS IS” BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
/**
 *  Author     : Cesar Fuguet
 *  Date       : October, 2024
 *  Description: Class definition of the HPDCACHE test random sequence
 */
#ifndef __HPDCACHE_TEST_ZAC_SEQ_H__
#define __HPDCACHE_TEST_ZAC_SEQ_H__

#include "hpdcache_test_defs.h"
#include "hpdcache_test_mem_resp_model_base.h"
#include "hpdcache_test_sequence.h"
#include "scv.h"
#include <systemc>

class hpdcache_test_zac_seq : public hpdcache_test_sequence
{
public:
    hpdcache_test_zac_seq(sc_core::sc_module_name nm)
      : hpdcache_test_sequence(nm, "zac_seq")
    {
        SC_THREAD(run);
        sensitive << clk_i.pos();

        seg.set_base(0x00000000ULL);
        seg.set_length(0x00004000ULL); // 2^14
        seg.set_uncached(false);

        hpdcache_test_sequence::segptr->keep_only(0);
        hpdcache_test_sequence::delay->keep_only(0);
        
        hpdcache_test_sequence::op_distribution.push(
            hpdcache_test_transaction_req::HPDCACHE_REQ_LOAD, 50);
        hpdcache_test_sequence::op_distribution.push(
            hpdcache_test_transaction_req::HPDCACHE_REQ_STORE, 50);
        hpdcache_test_sequence::op->set_mode(op_distribution);
    }

private:
    typedef sc_bv<HPDCACHE_REQ_DATA_WIDTH> req_data_t;

    hpdcache_test_sequence::hpdcache_test_memory_segment seg;
    scv_smart_ptr<req_data_t> data;
    scv_smart_ptr<req_data_t> size;
    const unsigned int HPDCACHE_REQ_DATA_BYTES = HPDCACHE_REQ_DATA_WIDTH / 8;

    int NADDR = 3;
    int NDATA = 3;

#if SC_VERSION_MAJOR < 3
    SC_HAS_PROCESS(hpdcache_test_zac_seq);
#endif

    inline req_data_t create_random_data()
    {
        data->keep_only(1,NDATA); //
        data->next();
        return data->read();
    }

    inline uint32_t create_random_size(bool is_amo)
    {
        if (!is_amo) size->keep_only(3, 3); // (0,3)
        // 3 car 2^3=8 bytes dans le CONF_HPDCACHE_WORD_WIDTH*CONF_HPDCACHE_CL_WORDS=64
        else size->keep_only(2, 3);

        size->next();
        return size->read().to_uint();
    }

    void send_transaction(std::shared_ptr<hpdcache_test_transaction_req> t)
    {
        // send transaction to the driver
        transaction_fifo_o->write(t);

        // wait and consume driver acknowledgement (this is blocking)
        transaction_ret_i.read();

        // release the previously used transaction object
        release_transaction<hpdcache_test_transaction_req>(t);

        // add some random delay between two consecutive commands
        for (int i = 0; i < delay->read(); i++) {
            wait();
        }
    }

    std::shared_ptr<hpdcache_test_transaction_req> create_random_transaction()
    {
        std::shared_ptr<hpdcache_test_transaction_req> t;

        scv_smart_ptr<uint64_t> addr("addr");
        scv_bag<uint64_t> address_distribution;
        for (int i=1; i<=NADDR; i++){
            address_distribution.push(i << 12); // 0x00001000ULL 2000 3000
        }
        addr->set_mode(address_distribution);
        addr->next();

        while (!is_available_id()) wait();

        hpdcache_test_sequence::segptr->next();
        hpdcache_test_sequence::delay->next();
        hpdcache_test_sequence::op->next();

        t = acquire_transaction<hpdcache_test_transaction_req>();        
        t->req_op = op->read();
        t->req_wdata = (t->is_store()) ? create_random_data() : 0;
        t->req_sid = 0;
        t->req_tid = allocate_id();
        t->req_abort = false;
        t->req_phys_indexed = false;

        t->req_wr_policy_hint = hpdcache_test_transaction_req::HPDCACHE_WR_POLICY_WT;

        uint32_t sz = create_random_size(t->is_amo() || t->is_amo_sc() || t->is_amo_lr()); // 3

        uint64_t length = seg.get_length();
        uint32_t bytes = 1 << sz;
        uint64_t address = ((addr->read() % length) / bytes) * bytes;
        uint32_t offset = address % HPDCACHE_REQ_DATA_BYTES;

        t->req_addr = address;
        t->req_be = ((1UL << bytes) - 1) << offset;
        t->req_size = sz;
        t->req_uncacheable = seg.is_uncached() ? 1 : 0;
        t->req_need_rsp = true;

        return t;
    }

    void run()
    {
        while (rst_ni == 0) wait();

        std::shared_ptr<hpdcache_test_mem_resp_model_base> mem_resp_model =
            this->get_mem_resp_model();

        scv_bag<pair<int, int>> zero_delay_distribution;

        zero_delay_distribution.push(pair<int, int>(0, 0), 100);

        mem_resp_model->set_ra_ready_delay_distribution(zero_delay_distribution);
        mem_resp_model->set_rd_valid_delay_distribution(zero_delay_distribution);
        mem_resp_model->set_wa_ready_delay_distribution(zero_delay_distribution);
        mem_resp_model->set_wd_ready_delay_distribution(zero_delay_distribution);
        mem_resp_model->set_wb_valid_delay_distribution(zero_delay_distribution);

        wait();

        for (size_t n = 0; n < this->max_transactions; n++) {
            std::shared_ptr<hpdcache_test_transaction_req> t;
            t = create_random_transaction();
            send_transaction(t);
        }

        //  ask the driver to stop
        transaction_fifo_o->write(nullptr);
    }
};

#endif // __HPDCACHE_TEST_ZAC_SEQ_H__
