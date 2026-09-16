/****
* Testbench de simulation à partir d'une suite d'actions.
* Preprocess : fonction "call_input", avec map des string vers les dut->fields.
****/

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>
// #include <verilated.h>
// #include <verilated_vcd_c.h>
// #include "V%NOM_MODULE%.h"
// %INCLUDE_024UNIT%

#include <vector>
using namespace std;

#include "tb.hpp"

// imports présents dans bcg_execute
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "testor_labels.h"

#define streq(s1,s2) (strcmp ((s1), (s2)) == 0)

// /* masque pour identifier les etiquettes correspondant a une entree */
// static CAESAR_TYPE_HIDE_1 match_input = NULL;
// #define IS_INPUT(L) CAESAR_TEST_HIDE_1 (match_input, (L))

/* The following function prints information about a BCG graph */
void bcg_print_info (BCG_TYPE_OBJECT_TRANSITION bcg_graph)
{
   printf ("initial state = %lu\n", BCG_OT_INITIAL_STATE (bcg_graph));
   printf ("nb states = %lu\n", BCG_OT_NB_STATES (bcg_graph));
   printf ("nb edges = %lu\n", BCG_OT_NB_EDGES (bcg_graph));
   printf ("nb labels = %u\n", BCG_OT_NB_LABELS (bcg_graph));
}

// static BCG_TYPE_C_STRING tool;
// #define ABORT(...) \
// 	{ \
// 		fprintf (stderr, "%s%s: error: ", tool, " (TC)"); \
// 		fprintf (stderr, __VA_ARGS__); \
// 		fprintf (stderr, "\n"); \
// 		exit (1); \
// 	}

// issu de seq_to_testbench/seq_parser.cpp
string trim(string line, vector<char> v) {
    string line2 = line;
    size_t first, last;
    do {
        line = line2;
        for (char c: v) {
            first = line2.find_first_not_of(c);
            if (first == string::npos) {
                return "";
            };
            last = line2.find_last_not_of(c);
            line2 = line2.substr(first, (last - first + 1));
        };
    } while (line != line2);
    return line2;
}

// provient de seq_to_testbench/seq_parser.hpp :
// Divise str en plusieurs substrings délimités par delim, et les entre dans le vecteur elems.
vector<string> split(string str, char delim)
{
    vector<string> elems;
    auto pos = str.find(delim);
    while (pos != string::npos) {
        elems.push_back(str.substr(0, pos));
        str.erase(0, pos + 1);
        pos = str.find(delim);
    }
    elems.push_back(str);
    return elems;
}

// Convertit un "string" en "char*". Type BCG_TYPE_C_STRING, utile pour streq ou IS_INPUT.
char* stocc (string str){
    return const_cast<char*>(str.data());
}

// struct Action {
//     string gate;
//     string offer;
// };

// Parse un label "GATE!offer", renvoie GATE et offer.
Action trimAction (BCG_TYPE_C_STRING str){
    vector<string> act = split(str, '!');
    string gate = trim(act[0], {' '}); // enlever l'espace
    string offer = (act.size() == 2) ? trim(act[1], {' '}) : "";
    return {gate, offer};
}


// map<string, void*> fields;

// // provient de seq_to_testbench/tb_template_2M :
// // Attribue à une porte une valeur d'offre.
// void call_input (V%NOM_MODULE% *&dut, string porte, string offre){

//     if (offre == "FALSE") {
//         *(bool*)(fields[porte]) = false;
//     } else if (offre == "TRUE") {
//         *(bool*)(fields[porte]) = true;
//     %TYPEDEF_ENUMS%
//     } else {
//         try {
//             *(unsigned char*)(fields[porte]) = stoi(offre);
//         }
//         catch (string& s) {
//             std::cout << "ERREUR en lisant la ligne" <<  "\'" << porte << " " << offre << ": \"" << s << "\"" << endl;
//             exit(1);
//         }
//     }
// };


// // provient de seq_to_testbench/tb_template_2M :
// // S'assure qu'une porte a bien une valeur d'offre.
// bool assert_output (V%NOM_MODULE% *&dut, string porte, string offre){

//     if (offre == "FALSE") {
//         return (*(bool*)(fields[porte]) == false);
//     } else if (offre == "TRUE") {
//         return (*(bool*)(fields[porte]) == true);
//     } else {
//         return (*(unsigned char*)(fields[porte]) == stoi(offre));
//     };
// }


// provient de vcd_to_seq/seq_generator.hpp :
// Correspondance entre portes LNT et signaux SV, à partir d'un fichier .gates.

// nom de signal & ses abstractions
// struct data_gate {
//     string name = "";
//     map<string, string> abstraction = {};
// };

// struct gate_match {
//     string gate = ""; // Name in formal model
//     string output = "";
//     string input = "";
//     vector<data_gate> datas = {};
// };

// using vec_gates = vector<gate_match>;


vec_gates parseGATESFile(string filename){
    ifstream file (filename + ".gates");
    
    if (file.is_open())
    {
        string line;
        vec_gates v;
        gate_match current;
        
        vector<string> abs;
        while (getline (file, line))
        {
            if (line.length() == 0) {                
                v.push_back(current);
                current = {};
            } else {
                string s = line.substr(2,line.length()-1);
                switch (line[0]){
                    case 'n':
                        if (current.gate != "") {
                            throw string("gate name already defined : ") + current.gate + " " + s + " (" + filename + " )";
                        }
                        current.gate = s;
                        break;
                    
                    case 'o':
                        current.output = s;
                        break;
                    
                    case 'i':
                        current.input = s;
                        break;
                    
                    case 'd':
                        current.datas.push_back({s, {}});
                        break;
                    
                    case '-': // abstractions
                        abs = split(s, ' ');
                        current.datas.back().abstraction[abs[0]] = abs[1];
                        break;

                    default:
                        throw string("caractère inconnu dans fichier io : ") + filename;
                        break;
                }
                
            }
        }
        if ((current.gate + current.output + current.input != "") || (current.datas.size() > 0)) { // last gate
            v.push_back(current);
        }
        file.close();
        return v;
    }
    else
    {
        throw string("erreur lecture fichier io : ") + filename;
    }
}

uint64_t sim_time; //
// // Si les _o sont bons, envoyer les _i
// bool do_action (V%NOM_MODULE% *&dut, BCG_TYPE_C_STRING s, vec_gates v, VerilatedVcdC *&m_trace) {
    
//     Action act = trimAction(s);

//     for (auto const &[gate, output, input, data, type] : v) {
//         if (gate == act.gate) {

//             if (type == "produce") {
//                 if (assert_output(dut, output, "1")) { // ready_o
//                     call_input(dut, input, "1"); // valid_i
//                     call_input(dut, data, act.offer); // data_i
//                     return true;
//                 } else {
//                     return false;
//                 }
                
//             } else if (type == "receive") {
//                 if (assert_output(dut, data, act.offer) && (assert_output(dut, output, "1"))){ // valid_o & data_o
//                     call_input(dut, input, "1"); // ready_i
//                     return true;
//                 } else {
//                     return false;
//                 }
//             } else if (type == "clock") {
                
//                 call_input(dut, input, "1");
//                 dut->eval();
//                 m_trace->dump(sim_time); sim_time++;
//                 call_input(dut, input, "0"); // 1->0
//                 dut->eval();
//                 m_trace->dump(sim_time); sim_time++;
//                 call_input(dut, input, "1"); // 0->1
//                 dut->eval();
//                 for (auto const &[gate_, output_, input_, data_, type_] : v) {
//                     if (type_ == "reset") {
//                         call_input(dut, input_, "1");
//                     }
//                     else if (type_ != "clock") {
//                         call_input(dut, input_, "0"); // valeur par défaut
//                     }
//                 }
//                 return true;
//             }
//             throw string("type (produce/receive) non-précisé dans le .gates pour la porte " + gate);
//         }
//     }
//     return false;
// }


#define MAX_SIM_TIME 50

int tc_execute(BCG_TYPE_FILE_NAME tc_filename, int argc, char** argv) {
    // la partie de bcg_execute
    BCG_TYPE_OBJECT_TRANSITION bcg_graph;
    
    CAESAR_CREATE_HIDE_1 (&match_input,
        "hpdcache.io",
        "input",
        "output",
    2); // match_kind = gate

    BCG_INIT ();
    tool = argv[0];
    BCG_OT_READ_BCG_BEGIN (tc_filename, &bcg_graph, 1);
    bcg_print_info (bcg_graph);

    // la partie avec Verilator
    // V%NOM_MODULE% *dut = new V%NOM_MODULE%;
    vec_gates v = parseGATESFile("hpdcache");
    
    // %INPUT_BODY%
    // %OUTPUT_BODY%

    sim_time = 0;
    // Verilated::traceEverOn(true);
    // VerilatedVcdC *m_trace = new VerilatedVcdC;
    // dut->trace(m_trace, 5);
    // m_trace->open("%NOM_MODULE%.vcd");
    
    BCG_TYPE_STATE_NUMBER current = BCG_OT_INITIAL_STATE (bcg_graph);
    BCG_TYPE_C_STRING input = NULL;
    size_t length = 0;
    BCG_TYPE_EDGE_NUMBER nb_inputs;
    BCG_TYPE_LABEL_NUMBER label;
    BCG_TYPE_STATE_NUMBER next;
    BCG_TYPE_C_STRING s;
    BCG_TYPE_BOOLEAN found_output, found_successor;

    set<string> visited_outputs;

    while ( /* CONSTCOND */ 1 && sim_time < MAX_SIM_TIME) {
        nb_inputs = 0;
        BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {
            /*
            * recherche de verdicts ; dans un cas de test produit
            * par TESTOR, un etat de verdict est marque par une
            * transition self-loop
            */
            s = BCG_OT_LABEL_STRING (bcg_graph, label);
            if (streq (s, TESTOR_INCONCLUSIVE) || streq (s, TESTOR_PASS)) {
                /* verdict atteint */
                fprintf (stderr, "%s\n\n", s);
                fprintf (stdout, "%s\n\n", s);
                goto END;
            }
            if (!BCG_OT_LABEL_VISIBLE (bcg_graph, label))
                ABORT ("unexpected internal transition");
            if (IS_INPUT (BCG_OT_LABEL_STRING (bcg_graph, label)))
                nb_inputs++;
        } BCG_OT_END_ITERATE;

	    /* priorite 1 : emission de l'entrée */
        // assert (nb_inputs <= 1);
	    if (nb_inputs == 1) {
            BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {
                s = BCG_OT_LABEL_STRING (bcg_graph, label);
                if (IS_INPUT (s)) {
                    // if (do_action (dut, s, v, m_trace)) {                   
                    //     Action act = trimAction(s);
                    //     fprintf (stdout, ">>> (%li->%li) action chosen: gate=%s, offer=%s\n", current, next, act.gate.c_str(), act.offer.c_str());
                    //     current = next;
                    //     break;
                    // }
		        }

                // pas d'autre transition donc finir en fail
                fprintf (stderr, ":FAIL: (impossible input \"%s\" in state #%lu)\n\n", s, current);
                fprintf (stdout, ":FAIL: (impossible input \"%s\" in state #%lu)\n\n", s, current);
                goto END;

	       } BCG_OT_END_ITERATE;
	       goto CONTINUE_WHILE; // skip à l'itération suivante dans le while -> état suivant
	    }

        /* priorite 2 : on regarde la liste des interfaces du DUT,
                        s'assurer que les outputables sont bien dans les successeurs du TC
                        et en prendre une s'il y en a */
        found_successor = BCG_FALSE;
        for (auto const &[gate, output, input, data] : v) {
            string type = ""; //
            if (type != "clock" && type != "reset" && !IS_INPUT (stocc(gate))
                     && !(visited_outputs.contains(gate))){ // && assert_output(dut, output, "1")
                string offre; // = to_string(*(unsigned char*)(fields[data]));

                // chercher s'il est bien dans les successeurs du test case
                found_output = BCG_FALSE;
                BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {
                    s = BCG_OT_LABEL_STRING (bcg_graph, label);
                    Action act = trimAction(s);                    
                    if ((gate == act.gate) && (offre == act.offer)) {
                        if (!found_successor) { // on avance qu'une seule fois
                            found_successor = BCG_TRUE;
                            // do_action (dut, s, v, m_trace);
                            visited_outputs.insert(gate);
                            fprintf (stdout, ">>> (%li->%li) action chosen: gate=%s, offer=%s\n", current, next, act.gate.c_str(), act.offer.c_str());
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
                    goto END;
                }
            }
        }
        if (found_successor)
            continue; // skip à l'itération suivante dans le while -> état suivant
        
        /* priorite 3 : emprunter le clock */
        
        for (auto const &[gate, output, input, data] : v) {
            string type = ""; //
            if (type == "clock") {
                BCG_OT_ITERATE_P_LN (bcg_graph, current, label, next) {
                    s = BCG_OT_LABEL_STRING (bcg_graph, label);
                    if (streq (s, stocc(gate))) {
                        found_successor = BCG_TRUE;
                        // do_action (dut, stocc(gate), v, m_trace);
                        visited_outputs.clear();
                        fprintf (stdout, ">>> (%li->%li) %s\n", current, next, gate.c_str());
                        current = next;
                        break;
                    }
                } BCG_OT_END_ITERATE;
            }
        }
        if (found_successor)
            continue; // skip à l'itération suivante dans le for -> interface suivante

        // trouvé aucun successeur adéquat et aucun clock
        fprintf (stderr, ":FAIL: (no output from state #%lu)\n\n", current);
        fprintf (stdout, ":FAIL: (no output from state #%lu)\n\n", current);
        goto END;

        CONTINUE_WHILE:
    }

    END:    

        // m_trace->close();
        BCG_OT_READ_BCG_END (&bcg_graph);
        // delete dut;
        exit(EXIT_SUCCESS);
}


int main(int argc, char** argv, char** env) {

    if (argc > 1){        
        string tc = argv[1];
        if (tc.substr(tc.size() - 4) == ".bcg") {
            fprintf (stdout, "-- EXÉCUTION DU TEST CASE \"%s\"\n", argv[1]);
            return tc_execute(argv[1], argc, argv);
        }
    }
    ABORT ("no testcase provided");
}
