MODULE=mau
SV_DIR=../compositions/sv/
TC_NAME=tc_liveness.bcg

############ crée les test cases V1 (coverage)
coverage:
	lnt.open $(MODULE)_tb generator $(MODULE)
	bcg_info $(MODULE)
	svl coverage -gate $(MODULE).bcg -gate $(MODULE).io

# ça en option ?
	mkdir -p bcgs
	mv $(MODULE)*.bcg COVER*.bcg CTG*.bcg INCONCLUSIVE.bcg REVERSE_COVER*.bcg TP*.bcg bcgs


############ exemple de bcg_execute sur le test case
define tc_execute_script =
gcc -I$CADP/incl Bcg_Execute/bcg_execute.c -L$CADP/bin.x64 -lcaesar -lBCG -lcaesar_plug -o bcg_execute2
for file in TC.CTG*.*.bcg; do
    ./bcg_execute2 -io alu.io -timeout 10 $file
done
endef
# importer le $MODULE !

tc_execute: ; $(value tc_execute_script)

.ONESHELL:

############
define info_script =
for file in *.bcg; do
    bcg_info "$(basename "$file")"
done
endef

info: ; $(value info_script)

.ONESHELL:

############ crée les test cases V2 (test purposes AUT)
build_tp:
	bcg_io tp_liveness.aut tp_liveness.bcg
	bcg_io tp_safety.aut tp_safety.bcg

build_tc: build_tp ../compositions/lnt/mau2_hide
	bcg_open ../compositions/lnt/mau2_hide testor -io -gate ${MODULE}.io tp_liveness.bcg tc_liveness.bcg
	bcg_open ../compositions/lnt/mau2_hide testor -io -gate ${MODULE}.io -all tp_liveness.bcg tc_ctg_liveness.bcg
	bcg_open ../compositions/lnt/mau2_hide testor -io -gate ${MODULE}.io tp_safety.bcg tc_safety.bcg
	bcg_open ../compositions/lnt/mau2_hide testor -io -gate ${MODULE}.io -all tp_safety.bcg tc_ctg_safety.bcg 
	rm -f tp_*.o testor

build_tc_all: build_tp
	extract_all -check -copy -io -gate ${MODULE}.io tc_ctg_liveness.bcg tc_liveness
	extract_all -check -copy -io -gate ${MODULE}.io tc_ctg_safety.bcg tc_safety

############ compile et exécute le testbench SV en explorant un test case

build_tb:
	./tb_PREPROC.sh ${MODULE} ${SV_DIR}

verilate: build_tb
	@echo
	@echo "### VERILATING ###"
	verilator -CFLAGS "-std=c++23 -I${CADP}/incl -L${CADP}/bin.x64 -lcaesar -lBCG -lBCG_IO -lcaesar_plug -lm -Wno-write-strings" -LDFLAGS "-I${CADP}/incl -L${CADP}/bin.x64 -lcaesar -lBCG -lBCG_IO -lcaesar_plug -lm" --Wall --trace --x-assign 0 --x-initial 0 -Wno-UNUSEDSIGNAL $(PARAMS_$(MODULE)) -cc -I${SV_DIR} $(MODULE).sv --exe tb_explore_$(MODULE).cpp

build: verilate
	@echo
	@echo "### BUILDING SIM ###"
	make -C obj_dir -f V$(MODULE).mk V$(MODULE)

sim:
	@echo
	@echo "### SIMULATING ###"
	./obj_dir/V$(MODULE) ${TC_NAME}

waves:
	gtkwave $(MODULE).vcd -a $(MODULE)_setup.gtkw


############
clean:
	svl -sweep
	rm -f *.c *.f *.lib *.lotos *.o *.t *.ps
	rm -f generator ocis testor
	rm -rf obj_dir
	rm -f tb_explore_*.cpp *.vcd
	rm -f tp_*.bcg tc_*.bcg
