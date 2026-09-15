#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include "utils.hpp"
#include <map>


string scopesig(id_s i){
    return i.scope + "." + i.reference;
}

ostream& operator<<(ostream &os, map_ids &ID)
{
    for (auto const &[k, v] : ID)
        os << k << " - " << "(type=" << v.var_type << ", size=" << v.size << ", ref=" << v.reference << ", scope=" << v.scope << ")" << endl;
    return os;
}


ostream& operator<<(ostream &os, changes_id ci)
{
    for(int i(0); i<ci.change_times.size(); ++i) { // timesteps
        // if (ci.change_times[i].changes.size() > 1) {
            // on assume qu'un changes contient au moins le clk
            os << "## T=" << ci.change_times[i].t << endl;
            for (auto const &[id, val] : ci.change_times[i].changes)
                os << "(ref=" << ci.ID[id].reference << ", new_val=" << val << ")" << endl;
    }
    return os;
}

string toUpper(string str)
{
    string str_up = str;
    transform(str_up.begin(), str_up.end(), str_up.begin(), ::toupper);
    return str_up;
}

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


ostream& operator<<(ostream &os, data_gate &d)
{
    os << d.name << " (";    
    for (auto it = d.abstraction.begin(); it != d.abstraction.end(); ++it)
    {
        os << it->first << "->" << it->second << " ";
    }
    os << ")";
    return os;
};

ostream& operator<<(ostream &os, vec_gates v){
    for(int i(0); i<v.size(); ++i)
        os << v[i].gate << " : output=" << v[i].output << ", input=" << v[i].input << ", datas=" << v[i].datas << endl;
    return os;
}

vec_gates parseGATESFile(string filename, bool verbose){
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
        if (verbose){
            cout << "gates : " << endl << v << endl;
        }
        file.close();
        return v;
    }
    else
    {
        throw string("erreur lecture fichier io : ") + filename;
    }
}
