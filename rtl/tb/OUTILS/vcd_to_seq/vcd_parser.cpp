
#include <algorithm>
#include <cassert>
#include "vcd_parser.hpp"

using namespace std;


ostream& operator<<(ostream &os, Port p)
{
    map<Port, string> PortStr = { {INPUT, "INPUT"}, {OUTPUT, "OUTPUT"} };
    os << PortStr[p];
    return os;
}

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

// ligne de la forme "$scope module TOP $end"
void parseScope(string line, Scope *&scope) // "pass a pointer by reference"
{
    vector<string> elems = split(line, ' ');
    scope = scope->insert(elems[2]);
}

// ligne de la forme "$var wire 1 $ in_valid $end"
void parseVar(string line, map_ids &ID, Scope *scope)
{
    vector<string> elems = split(line, ' ');
    char id = (elems[3]).front();

    if (ID.find(id) == ID.end()) { // s'il y est pas déjà
        ID[id] = {elems[1], stoi(elems[2]), elems[4], scope->get_name()};
    }
}

// ligne de la forme "#5"
void parseTime(string line, vec_changes &changes, timesteps *&cur_time)
{
    int t = stoi(line.substr(1, line.length()-1));
    changes.push_back({t, {}});
    cur_time = &(changes.back());
}

// ligne de la forme "b0001010 $"
void parseValChange(string line, timesteps *&cur_time, map_ids ID, set<string> scopes)
{
    char id = line.back();

    if (scopes.find(ID[id].scope) != scopes.end()){ // si le scope est bon
        string val = trim(line.substr(0, line.length()-1), {' '});
        (cur_time->changes)[id] = val;
    }
}


/*
void resetChangeFlag(map_tofill &mvc, Port p) {
    for (auto &[idd, v] : mvc)
        if (v.port == p)
            v.has_changed = false;
}


void fillChanges(int t, vec_changes &changes, map_tofill &mvc, Port p) {
    for (auto &[idd, v] : mvc) {
        if (v.port == p and !v.has_changed and v.current_val != "") { // ?
            // tout xput inchangé dans le cycle précédent, on le remplit
            changes.push_back({t, idd, v.current_val});
            v.has_changed = true;
        }
    }
}

bool isBalanced(vec_changes changes)
{
    map<int, int> counts;
    for (auto &[t, id, val] : changes) {
        counts[t]++;
    }
    for (auto &[t, cpt] : counts) {
        if (counts[t] != counts[0]) {
            cout << "isBalanced: " << t << " différent !" << endl;
            return false;
        }
    }
    return true;
}
*/

void parseVCDFile(string filename, map_ids &ID, vec_changes &changes, bool verbose)
{
    // 1 - parcourir le vcd, remplir ID et changes

    ifstream file (filename);

    if (file.is_open())
    {
        string line;
        timesteps* cur_time;

        Scope scope ("*");
        Scope* scope_ptr = &scope;


        while (getline (file, line))
        {
            line = trim(line, {' ', '\r'});

            // syntaxe VCD en section 21.7.2 :
            // les lignes commencent par '$', ou sont une value_change '#', ou sont un simulation_time.
            if (line.substr(0, 7) == "$scope ") {
                parseScope(line, scope_ptr);
            
            } else if (line.substr(0, 9) == "$upscope ") {
                scope_ptr = scope_ptr->get_parent();
            
            } else if (line.substr(0, 5) == "$var ") {
                parseVar(line, ID, scope_ptr);
            
            } else if (line[0] == '#') {
                parseTime(line, changes, cur_time);
            
            } else if (!line.empty() and line[0] != '$') {
                parseValChange(line, cur_time, ID, {"TOP"});
            }
        }
        
        if (verbose){            
            cout << endl << "hiérarchie des scopes : " << scope << endl << endl;
        }
        file.close();
    }
    else
    {
        throw string("erreur ouverture fichier lecture : ") + filename;
    }
}
