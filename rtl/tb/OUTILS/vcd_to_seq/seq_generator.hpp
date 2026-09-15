#ifndef SEQ_GENERATOR_INCLUDED
#define SEQ_GENERATOR_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include "utils.hpp"


struct gate_match {
    string gate = ""; // Name in formal model
    string output = "";
    string input = "";
    string data = "";
};

using vec_gates = vector<gate_match>;

ostream& operator<<(ostream &os, vec_gates v);

// Lit un fichier de correspondance entre portes LNT et signaux SV.
vec_gates parseGATESFile(string filename, bool verbose);

// Convertit une liste de changements vers une trace SEQ.
void generateSEQFile(string filename, map_ids ID, vec_changes changes, vec_gates v, bool verbose);

#endif
