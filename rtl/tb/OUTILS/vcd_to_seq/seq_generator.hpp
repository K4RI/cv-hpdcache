#ifndef SEQ_GENERATOR_INCLUDED
#define SEQ_GENERATOR_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include "utils.hpp"


// Convertit une liste de changements vers une trace SEQ.
void generateSEQFile(string filename, map_ids ID, vec_changes changes, vec_gates v, bool verbose);

#endif
