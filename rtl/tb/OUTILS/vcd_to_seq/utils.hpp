#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>

using namespace std;

struct id_s {
    string var_type;
    int size;
    string reference;
    string scope;
};

// Valeur associée à un identifiant : son type, sa taille, son vrai nom, son scope.
using map_ids = unordered_map<char, id_s>;

ostream& operator<<(ostream &os, map_ids &ID);

/*
struct change_s {
    int t;
    char id;
    string val;
};

// Informations sur les changements de valeur.
using vec_changes = vector<change_s>;
*/


struct timesteps {
    int t;
    map<char, string> changes;
};

using vec_changes = vector<timesteps>;


// Une struct uniquement créée pour l'operator<< avec deux sous-structures...
struct changes_id {
    vec_changes change_times;
    map_ids ID;
};

ostream& operator<<(ostream &os, changes_id ci);

template <typename T>
ostream& operator<<(ostream& os, const vector<T>& v) { // obligé de le def dans .hpp...
    // os << "(";
    for(int i(0); i<v.size(); ++i)
        os << v[i] << " " ;
    // os << ")";
    return os;
};

// ostream& operator<<(ostream &os, vector<char> v);

// Met un mot en majuscule.
string toUpper(string str);

// Divise str en plusieurs substrings délimités par delim, et les entre dans le vecteur elems.
vector<string> split(string str, char delim);

#endif
