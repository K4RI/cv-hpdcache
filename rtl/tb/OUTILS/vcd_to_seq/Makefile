CC=g++
TRACE=../../logs/run_zac_42

######## traduction vcd->seq ########

main:
	@echo
	@echo "### COMPILATION TRADUCTEUR VCD-SEQ ###"
	$(CC) -std=c++20 main.cpp vcd_parser.cpp seq_generator.cpp Scope.cpp utils.cpp -o main

sim_seq: main
	@echo
	@echo "### TRADUCTION VCD-SEQ ###"
	./main ${TRACE}.vcd 1

show_seq: sim_seq
# convertir et visualiser bcg
	@echo
	@echo "### AFFICHAGE SEQ ###"
	bcg_io ${TRACE}.seq ${TRACE}.bcg
	bcg_edit ${TRACE}.bcg


################################

inclusion:
	bcg_open ../compositions/lnt/${MODULE}2_hide exhibitor < ${TRACE}.seq

clean:	
	rm -f *.bcg *.o *.ps main
	rm -f exhibitor evaluator*
	rm -f *.vcd *.seq
