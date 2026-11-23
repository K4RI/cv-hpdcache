/**
 *  Copyright 2023,2024 Commissariat a l'Energie Atomique et aux Energies Alternatives (CEA)
 *  Copyright 2025 Inria, Universite Grenoble-Alpes, TIMA
 *
 *  SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 */
/**
 *  Author     : Cesar Fuguet
 *  Date       : October, 2024
 *  Description: class definition of the HPDCACHE test driver
 */
#ifndef __HPDCACHE_TEST_DRIVER_H__
#define __HPDCACHE_TEST_DRIVER_H__

#include "driver.h"
#include "hpdcache_test_defs.h"
#include "hpdcache_test_transaction.h"
#if CONF_HPDCACHE_TEST_FAULT_INJ
#include "hpdcache_fault_injection.h"
#endif
#include "logger.h"
#include <string>
#include <systemc>
#include <verilated.h>

#include "hpdcache_test_sequence.h"
// #include "sequence.h"
// #include "transaction_pool.h"
// acquire_transaction(), allocate_id(). TODO ça include mais ça les reconnaît pas ??
#include "tb.hpp"

using namespace sc_core;
using namespace sc_dt;

class hpdcache_test_driver : public Driver
{
public:
    sc_out<bool> core_req_valid_o;
    sc_in<bool> core_req_ready_i;
    sc_out<sc_bv<HPDCACHE_CORE_REQ_WIDTH>> core_req_o;
    sc_out<bool> core_req_abort_o;
    sc_out<sc_bv<HPDCACHE_TAG_WIDTH>> core_req_tag_o;
    sc_out<sc_bv<HPDCACHE_REQ_PMA_WIDTH>> core_req_pma_o;

    sc_in<bool> core_rsp_valid_i;
    sc_in<sc_bv<HPDCACHE_CORE_RSP_WIDTH>> core_rsp_i;
    sc_in<bool> no_inflight_requests_i;

    sc_fifo_out<hpdcache_test_transaction_req> sb_core_req_o;
    sc_fifo_out<hpdcache_test_transaction_resp> sb_core_resp_o;

    // #define FORMAL_TC
    
    #ifdef FORMAL_TC //////////////////////////////////////// EXPLORATION TEST CASE
    CAESAR_TYPE_HIDE_1 match_input = NULL; // static
    #define IS_INPUT(L) CAESAR_TEST_HIDE_1 (match_input, (L))

    char* tc_filename = stocc("tc_consistency.bcg");
    BCG_TYPE_OBJECT_TRANSITION bcg_graph;

    vec_gates v = parseGATESFile("hpdcache");
    
    BCG_TYPE_C_STRING input = NULL;
    size_t length = 0;
    BCG_TYPE_EDGE_NUMBER nb_inputs;
    BCG_TYPE_LABEL_NUMBER label;
    BCG_TYPE_STATE_NUMBER current;
    BCG_TYPE_STATE_NUMBER next;
    BCG_TYPE_C_STRING s;
    BCG_TYPE_BOOLEAN found_output, found_successor;
    const unsigned int HPDCACHE_REQ_DATA_BYTES = HPDCACHE_REQ_DATA_WIDTH / 8;
    #endif ////////////////////////////////////////


    hpdcache_test_driver(sc_core::sc_module_name nm)
      : Driver(nm)
    {
        SC_THREAD(drive_request);
        sensitive << clk_i.pos();

        SC_THREAD(monitor_response);
        sensitive << clk_i.neg();

        #ifdef FORMAL_TC
        ////////////
        CAESAR_CREATE_HIDE_1 (&match_input,
            (char*) "hpdcache.io", // TODO
            (char*) "input",
            (char*) "output",
        2);
        
        BCG_INIT ();
        BCG_OT_READ_BCG_BEGIN (tc_filename, &bcg_graph, 1);
        bcg_print_info (bcg_graph);
        current = BCG_OT_INITIAL_STATE (bcg_graph);
        ////////////
        #endif
    };

private:
#if SC_VERSION_MAJOR < 3
    SC_HAS_PROCESS(hpdcache_test_driver);
#endif

    hpdcache_fault_injection faultInj;

    typedef std::shared_ptr<hpdcache_test_transaction_req> transaction_ptr;

    static inline uint64_t core_get_req_tag(const transaction_ptr& t)
    {
        sc_bv<HPDCACHE_TAG_WIDTH> ret;
        ret = t->req_addr.range(HPDCACHE_PA_WIDTH - 1, HPDCACHE_ADDR_OFFSET_WIDTH);
        return ret.to_uint64();
    }

    static inline uint32_t core_get_req_pma(const transaction_ptr& t)
    {
        int ret = 0;
        if (t->is_wr_policy_auto()) ret |= (0x1 << 0);
        if (t->is_wr_policy_wb()) ret |= (0x1 << 1);
        if (t->is_wr_policy_wt()) ret |= (0x1 << 2);
        if (t->req_io) ret |= (0x1 << 3);
        if (t->req_uncacheable) ret |= (0x1 << 4);
        return ret;
    }

    static sc_bv<HPDCACHE_CORE_REQ_WIDTH> core_req_to_bv(const transaction_ptr& t)
    {
        const int unsigned PMA_POS = 0;
        const int unsigned TAG_POS = PMA_POS + HPDCACHE_REQ_PMA_WIDTH;
        const int unsigned PHYS_INDEXED_POS = TAG_POS + HPDCACHE_TAG_WIDTH;
        const int unsigned NEED_RSP_POS = PHYS_INDEXED_POS + HPDCACHE_REQ_PHYS_INDEXED_WIDTH;
        const int unsigned TRANS_ID_POS = NEED_RSP_POS + HPDCACHE_REQ_NEED_RSP_WIDTH;
        const int unsigned SRC_ID_POS = TRANS_ID_POS + HPDCACHE_REQ_TRANS_ID_WIDTH;
        const int unsigned SIZE_POS = SRC_ID_POS + HPDCACHE_REQ_SRC_ID_WIDTH;
        const int unsigned BE_POS = SIZE_POS + HPDCACHE_REQ_SIZE_WIDTH;
        const int unsigned OP_POS = BE_POS + (HPDCACHE_REQ_DATA_WIDTH / 8);
        const int unsigned WDATA_POS = OP_POS + HPDCACHE_REQ_OP_WIDTH;
        const int unsigned ADDR_POS = WDATA_POS + HPDCACHE_REQ_DATA_WIDTH;
        sc_bv<HPDCACHE_CORE_REQ_WIDTH> ret;

        ret.range(PMA_POS + HPDCACHE_REQ_PMA_WIDTH - 1, PMA_POS) = core_get_req_pma(t);
        ret.range(TAG_POS + HPDCACHE_TAG_WIDTH - 1, TAG_POS) = core_get_req_tag(t);
        ret.range(PHYS_INDEXED_POS + HPDCACHE_REQ_PHYS_INDEXED_WIDTH - 1, PHYS_INDEXED_POS) =
            t->req_phys_indexed ? 1 : 0;
        ret.range(NEED_RSP_POS + HPDCACHE_REQ_NEED_RSP_WIDTH - 1, NEED_RSP_POS) =
            t->req_need_rsp ? 1 : 0;
        ret.range(TRANS_ID_POS + HPDCACHE_REQ_TRANS_ID_WIDTH - 1, TRANS_ID_POS) = t->req_tid;
        ret.range(SRC_ID_POS + HPDCACHE_REQ_SRC_ID_WIDTH - 1, SRC_ID_POS) = t->req_sid;
        ret.range(SIZE_POS + HPDCACHE_REQ_SIZE_WIDTH - 1, SIZE_POS) = t->req_size;
        ret.range(BE_POS + (HPDCACHE_REQ_DATA_WIDTH / 8) - 1, BE_POS) = t->req_be;
        ret.range(OP_POS + HPDCACHE_REQ_OP_WIDTH - 1, OP_POS) = t->req_op;
        ret.range(WDATA_POS + HPDCACHE_REQ_DATA_WIDTH - 1, WDATA_POS) = t->req_wdata;
        ret.range(ADDR_POS + HPDCACHE_ADDR_OFFSET_WIDTH - 1, ADDR_POS) =
            t->req_addr.range(HPDCACHE_ADDR_OFFSET_WIDTH, 0);

        return ret;
    }

    static void core_resp_to_struct(hpdcache_test_transaction_resp& resp,
                                    const sc_bv<HPDCACHE_CORE_RSP_WIDTH>& bv)
    {
        const int unsigned ABORTED_POS = 0;
        const int unsigned ERROR_POS = ABORTED_POS + HPDCACHE_RSP_ABORTED_WIDTH;
        const int unsigned TRANS_ID_POS = ERROR_POS + HPDCACHE_RSP_ERROR_WIDTH;
        const int unsigned SRC_ID_POS = TRANS_ID_POS + HPDCACHE_REQ_TRANS_ID_WIDTH;
        const int unsigned RDATA_POS = SRC_ID_POS + HPDCACHE_REQ_SRC_ID_WIDTH;

        resp.rsp_aborted =
            (bv.range(ABORTED_POS + HPDCACHE_RSP_ABORTED_WIDTH - 1, ABORTED_POS).to_uint() > 0);
        resp.rsp_error =
            (bv.range(ERROR_POS + HPDCACHE_RSP_ERROR_WIDTH - 1, ERROR_POS).to_uint() > 0);
        resp.rsp_tid = bv.range(TRANS_ID_POS + HPDCACHE_REQ_TRANS_ID_WIDTH - 1, TRANS_ID_POS);
        resp.rsp_sid = bv.range(SRC_ID_POS + HPDCACHE_REQ_SRC_ID_WIDTH - 1, SRC_ID_POS);
        resp.rsp_rdata = bv.range(RDATA_POS + HPDCACHE_REQ_DATA_WIDTH - 1, RDATA_POS);
    }

    void drive_request()
    {
        transaction_ptr t;
        for (;;) {
            #ifndef FORMAL_TC
            t = std::dynamic_pointer_cast<hpdcache_test_transaction_req>(transaction_fifo_i.read());

            #else
            ////////////////////////////////////////
            // 0 - recherche de verdicts dans le TC
            nb_inputs = 0;
            BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {                
                s = BCG_OT_LABEL_STRING (bcg_graph, label);
                if (streq (s, TESTOR_INCONCLUSIVE) || streq (s, TESTOR_PASS)) {
                    // verdict atteint
                    fprintf (stderr, "%s\n\n", s);
                    fprintf (stdout, "%s\n\n", s);
                    bcg_end();
                }
                if (!BCG_OT_LABEL_VISIBLE (bcg_graph, label))
                    ABORT ("unexpected internal transition");
                if (IS_INPUT (BCG_OT_LABEL_STRING (bcg_graph, label)))
                    nb_inputs++;
            } BCG_OT_END_ITERATE;

            // 1 - recherche d'inputs successeurs CORE_REQ : continue
            
            if (nb_inputs == 1) {
                BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {
                    s = BCG_OT_LABEL_STRING (bcg_graph, label);
                    if (IS_INPUT (s)) { // il y a 1 input c'est la seule possibilité
                        
                        vector<string> act = split(s, '!');
                        // GATE !Load/Store !data !sid !tid !addr
                        string op = trim(act[1], {' '});
                        string data = act[2]; // TODO: substr enlever le D, attention au NoData donc ptet trim avant
                        string addr = act[5]; // pareil substr enlever le A, donc ptet trim avant

                        t = acquire_transaction<hpdcache_test_transaction_req>(); // transaction_pool

                        // cf les hpdcache_test_seq::create_random_transaction()
                        t->req_op = (op == "Load") ? 0 : 1; //
                        t->req_wdata = (data == "NoData") ? 0 : stoi(data); //
                        t->req_sid = 1;
                        t->req_tid = allocate_id();
                        t->req_abort = false;
                        t->req_phys_indexed = false;
                        t->req_wr_policy_hint = hpdcache_test_transaction_req::HPDCACHE_WR_POLICY_WT;
                        uint32_t sz = 3;
                        uint64_t length = 0x00004000ULL;
                        uint32_t bytes = 1 << sz;
                        uint64_t address = (((stoi(addr) << 12) % length) / bytes) * bytes; //
                        uint32_t offset = address % HPDCACHE_REQ_DATA_BYTES;
                        t->req_addr = address;
                        t->req_be = ((1UL << bytes) - 1) << offset;
                        t->req_size = sz;
                        t->req_uncacheable = 0;
                        t->req_need_rsp = true;
                        
                        fprintf (stdout, ">>> (%li->%li) action chosen: %s\n", current, next, s);
                        current = next;
                        break;
                    }

                    // pas d'autre transition donc finir en fail
                    fprintf (stderr, ":FAIL: (impossible input \"%s\" in state #%lu)\n\n", s, current);
                    fprintf (stdout, ":FAIL: (impossible input \"%s\" in state #%lu)\n\n", s, current);
                    bcg_end();
                } BCG_OT_END_ITERATE;
                continue;
            }
            ////////////////////////////////////////
            #endif

            if (t == nullptr) break;

            if (t->req_fault.valid) {
                switch (t->req_fault.domain) {
                    case hpdcache_fault_injection::domain_e::CACHE_DIR:
                        faultInj.injectDirFault(t->req_fault.set,
                                                t->req_fault.way,
                                                t->req_fault.fault_mask.range(63, 0));
                        break;
                    case hpdcache_fault_injection::domain_e::CACHE_DAT:
                        faultInj.injectDatFault(t->req_fault.set,
                                                t->req_fault.way,
                                                t->req_fault.word,
                                                t->req_fault.fault_mask.range(71, 0));
                        break;
                    default:
                        break;
                }
            }

            core_req_valid_o.write(true);
            core_req_o.write(core_req_to_bv(t));
            do wait();
            while (!core_req_ready_i.read());
            core_req_valid_o.write(false);

            // send the tag
            core_req_tag_o.write(core_get_req_tag(t));
            core_req_pma_o.write(core_get_req_pma(t));
            core_req_abort_o.write(t->is_aborted());

            transaction_ret_o.write(t->get_id());

            //  send request to the scoreboard
            sb_core_req_o.write(*t);
        }

        // FIXME : I should find a better way to know when all transactions
        // have been completed. Otherwise, finish the simulation in a different
        // place (e.g. the scoreboard that knows all pending transactions)
        do {
            wait();
        } while (!no_inflight_requests_i.read());

        Verilated::gotFinish(true);
    }

    void drive_tag()
    {
        for (;;) {

            wait();
        }
    }

    void monitor_response()
    {
        for (;;) {
            wait();

            if (core_rsp_valid_i.read()) {
                hpdcache_test_transaction_resp resp;
                core_resp_to_struct(resp, core_rsp_i.read());
                
                #ifdef FORMAL_TC
                ////////////////////////////////////////
                // 2 - est-ce que la réponse reçue est dans les successeurs ?
                
                string action = "CRI_RSP !" + (resp.rsp_rdata.to_string(SC_HEX)) + " !" + to_string(resp.rsp_sid.to_uint()) + " !" + to_string(resp.rsp_tid.to_uint()); // TODO : 1 -> A1, 0 -> NoData

                found_successor = BCG_FALSE;
                // chercher s'il est bien dans les successeurs du test case
                found_output = BCG_FALSE;
                BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {
                    s = BCG_OT_LABEL_STRING (bcg_graph, label);
                    if (s == action) {
                        if (!found_successor) { // on avance qu'une seule fois
                            found_successor = BCG_TRUE;
                            fprintf (stdout, ">>> (%li->%li) action chosen: %s\n", current, next, action.c_str());
                            current = next;
                        }
                        found_output = BCG_TRUE;
                        break; // sortir du BCG_OT_ITERATE_P_LN et passer à l'interface suivante
                    }
                } BCG_OT_END_ITERATE;
                if (!found_output){
                    // le DUT a une sortie non-prévue par le test case
                    fprintf (stderr, ":FAIL: (unexpected output \"%s\" in state #%lu)\n\n", s, current);
                    fprintf (stdout, ":FAIL: (unexpected output \"%s\" in state #%lu)\n\n", s, current);
                    bcg_end();
                }
                if (!found_successor) {                
                    // 3 - emprunter le clock       
                    // TODO: à mettre dans le else du if (core_rsp_valid_i.read()) ?         
                    for (auto const &[gate, output, input, data] : v) {
                        string type = ""; //
                        if (type == "clock") {
                            BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {
                                s = BCG_OT_LABEL_STRING (bcg_graph, label);
                                if (streq (s, stocc(gate))) {
                                    found_successor = BCG_TRUE;
                                    fprintf (stdout, ">>> (%li->%li) %s\n", current, next, gate.c_str());
                                    current = next;
                                    break;
                                }
                            } BCG_OT_END_ITERATE;
                        }
                    }
                }
                if (!found_successor) {
                    // trouvé aucun successeur adéquat et aucun clock
                    fprintf (stderr, ":FAIL: (no output from state #%lu)\n\n", current);
                    fprintf (stdout, ":FAIL: (no output from state #%lu)\n\n", current);
                    bcg_end();
                }
                ////////////////////////////////////////
                #endif

                //  send response to the scoreboard
                sb_core_resp_o.write(resp);
            }
        }
    }

    #ifdef FORMAL_TC
    void bcg_end() {
        BCG_OT_READ_BCG_END (&bcg_graph);
        exit(EXIT_SUCCESS);
    }
    #endif
};

#endif // __HPDCACHE_TEST_DRIVER_H__
// vim: ts=4 : sts=4 : sw=4 : et : tw=100 : spell : spelllang=en : fdm=marker
