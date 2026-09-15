#ifndef PARSER_INCLUDED
#define PARSER_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include "utils.hpp"
#include "Scope.hpp"


enum Port {
    INPUT,
    OUTPUT
};

ostream& operator<<(ostream &os, Port p);

struct fill_info {
    Port port;
    string current_val;
    bool has_changed;
};

// Valeur associée à un identifiant : input ou output, sa valeur en cours, a-t-elle été modifiée.
using map_tofill = unordered_map<char, fill_info>;

// Retire les caractères de v au début & à la fin d'un string.
string trim(string line, vector<char> v);

// 0 : commence par $scope. Change le scope des prochaines déclarations de variables.
void parseScope(string line, Scope *&scope);

// 1 : commence par $var. Entre dans le dictionnaire les informations de l'identifiant.
void parseVar(string line, map_ids & ID, Scope *scope);

// 2 : commence par #. Incrémente le temps de simulation.
void parseTime(string line, vec_changes &changes, timesteps *&cur_time);

// 3 : commence par tout sauf $. Note tout changement de valeur dans l'un des scopes notés.
void parseValChange(string line, timesteps *&cur_time, map_ids ID, set<string> scopes);

// fill : est-ce qu'on remplit les valeurs non-modifiées
void parseVCDFile(string filename, map_ids &ID, vec_changes &changes, bool verbose);


#endif // PARSER_INCLUDED
