#include "bcg_user.h"
#include "caesar_hide_1.h"

/* masque pour identifier les etiquettes correspondant a une entree */
static CAESAR_TYPE_HIDE_1 match_input = NULL;
#define IS_INPUT(L) CAESAR_TEST_HIDE_1 (match_input, (L))

/* The following function prints information about a BCG graph */
void bcg_print_info (BCG_TYPE_OBJECT_TRANSITION bcg_graph);

static BCG_TYPE_C_STRING tool;
#define ABORT(...) \
	{ \
		fprintf (stderr, "%s%s: error: ", tool, " (TC)"); \
		fprintf (stderr, __VA_ARGS__); \
		fprintf (stderr, "\n"); \
		exit (1); \
	}

// issu de seq_to_testbench/seq_parser.cpp
string trim(string line, vector<char> v);

// provient de seq_to_testbench/seq_parser.hpp :
// Divise str en plusieurs substrings délimités par delim, et les entre dans le vecteur elems.
vector<string> split(string str, char delim);

// Convertit un "string" en "char*". Type BCG_TYPE_C_STRING, utile pour streq ou IS_INPUT.
char* stocc (string str);

struct Action {
    string gate;
    string offer;
};

// Parse un label "GATE!offer", renvoie GATE et offer.
Action trimAction (BCG_TYPE_C_STRING str);

// nom de signal & ses abstractions
struct data_gate {
    string name = "";
    map<string, string> abstraction = {};
};

struct gate_match {
    string gate = ""; // Name in formal model
    string output = "";
    string input = "";
    vector<data_gate> datas = {};
};

using vec_gates = vector<gate_match>;


vec_gates parseGATESFile(string filename);