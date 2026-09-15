
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
    string id = elems[3];

    if (ID.find(id) == ID.end()) { // s'il y est pas déjà
        cout << elems[3] << " - " << elems[1] << " " << stoi(elems[2]) << " " << elems[4] << " - " << scope->get_name() << endl;
        ID[id] = {elems[1], stoi(elems[2]), elems[4], scope->get_name()};
    }
}

// ligne de la forme "#5"
void parseTime(string line, vec_changes &changes, timesteps *&cur_time)
{
    int t = stoi(line.substr(1, line.length()-1));
    changes.push_back({t, {}});
    cur_time = &(changes.back());
    cout << cur_time->t << endl;
}


// ligne de la forme "b0001010 $"
void parseValChange(string line, timesteps *&cur_time, map_ids ID, set<string> scopesigs)
{
    string id, val;
    if (line[0] == 'b') {
        vector<string> elems = split(line, ' ');
        val = elems[0];
        id = elems[1];
    } else {
        val = line[0];
        id = line.substr(1, line.length()-1);
    }

    if (scopesigs.find(scopesig(ID[id])) != scopesigs.end())
        (cur_time->changes)[id] = val;
}

set<string> set_of_gates(vec_gates v){
    set<string> s = {};    
    for (auto &[gate, output, input, datas] : v) {
        s.insert({output, input});
        for (data_gate & d : datas) {
            s.insert(d.name);
        }
    }
    s.erase("");
    return s;
}


void parseVCDFile(string filename, map_ids &ID, vec_changes &changes, vec_gates v, bool verbose)
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
                parseValChange(line, cur_time, ID, set_of_gates(v));

                // parseValChange(line, cur_time, ID, {"hpdcache_wrapper.clk_i",
                // "hpdcache_wrapper.core_req_valid_i",
                // "hpdcache_wrapper.core_req_ready_o",
                // "core_req_i.op",
                // "core_req_i.wdata[0]",
                // "core_req_i.sid",
                // "core_req_i.tid",
                // "core_req_i.addr_tag",
                // "hpdcache_wrapper.core_rsp_valid_o",
                // "core_rsp_o.rdata[0]",
                // "core_rsp_o.sid",
                // "core_rsp_o.tid"});

                // parseValChange(line, cur_time, ID, {"#", "&", "5)", "*", "(", "-", ".", "1", "6)", "7)", "9)", ":)"});
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
