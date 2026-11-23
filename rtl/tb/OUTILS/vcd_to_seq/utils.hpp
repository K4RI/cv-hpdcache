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

// scope.reference, par ex "core_req_i.op"
string scopesig(id_s i);

// Valeur associée à un identifiant : son type, sa taille, son vrai nom, son scope.
using map_ids = unordered_map<string, id_s>;

ostream& operator<<(ostream &os, map_ids &ID);

struct timesteps {
    int t;
    map<string, string> changes;
};

using vec_changes = vector<timesteps>;


// Une struct uniquement créée pour l'operator<< avec deux sous-structures : vector<timesteps> et map_ids
struct changes_id {
    vec_changes change_times;
    map_ids ID;
};

ostream& operator<<(ostream &os, changes_id ci);

template <typename T>
ostream& operator<<(ostream& os, vector<T>& v) { // obligé de le def dans .hpp...
    os << "(";
    for(int i(0); i<v.size(); ++i)
        os << v[i] << " " ;
    os << ")";
    return os;
};

// ostream& operator<<(ostream &os, vector<char> v);

// Met un mot en majuscule.
string toUpper(string str);

// Divise str en plusieurs substrings délimités par delim, et les entre dans le vecteur elems.
vector<string> split(string str, char delim);

// nom de signal & ses abstractions
struct data_gate {
    string name = "";
    map<string, string> abstraction = {};
};

ostream& operator<<(ostream &os, data_gate &d);

struct gate_match {
    string gate = ""; // Name in formal model
    string output = "";
    string input = "";
    vector<data_gate> datas = {};
};

using vec_gates = vector<gate_match>;

ostream& operator<<(ostream &os, vec_gates v);

// Lit un fichier de correspondance entre portes LNT et signaux SV.
vec_gates parseGATESFile(string filename, bool verbose);


#endif
