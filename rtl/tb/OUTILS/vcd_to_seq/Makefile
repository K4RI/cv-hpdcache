CC=g++
MODULE=mau

######## traduction vcd->seq ########

main:
	@echo
	@echo "### COMPILATION TRADUCTEUR VCD-SEQ ###"
	$(CC) -std=c++20 main.cpp vcd_parser.cpp seq_generator.cpp Scope.cpp utils.cpp -o main

sim_seq: main
	@echo
	@echo "### TRADUCTION VCD-SEQ ###"
	./main wave_R_${MODULE}.vcd 1

show_seq: sim_seq
# convertir et visualiser bcg
	@echo
	@echo "### AFFICHAGE SEQ ###"
	bcg_io wave_R_${MODULE}.seq wave_R_${MODULE}.bcg
	bcg_edit wave_R_${MODULE}.bcg


################################

inclusion:
	bcg_open ../compositions/lnt/${MODULE}2_hide exhibitor < wave_R_${MODULE}.seq

clean:	
	rm -f *.bcg *.o *.ps main
	rm -f exhibitor evaluator*
	rm -f *.vcd *.seq
