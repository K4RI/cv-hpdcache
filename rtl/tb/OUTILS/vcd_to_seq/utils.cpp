#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include "utils.hpp"
#include <map>


ostream& operator<<(ostream &os, map_ids &ID)
{
    for (auto const &[k, v] : ID)
        if (v.scope == "TOP")
        os << k << " - " << "(type=" << v.var_type << ", size=" << v.size << ", ref=" << v.reference << ", scope=" << v.scope << ")" << endl;
    return os;
}

/*
ostream& operator<<(ostream &os, changes_id ci)
{
    for(int i(0); i<ci.changes.size(); ++i)
        if (ci.ID[ci.changes[i].id].scope == "TOP")
        os << "(t=" << ci.changes[i].t << ", ref=" << ci.ID[ci.changes[i].id].reference << ", new_val=" << ci.changes[i].val << ")" << endl;
    return os;
}
*/

ostream& operator<<(ostream &os, changes_id ci)
{
    for(int i(0); i<ci.change_times.size(); ++i) { // timesteps
        os << "## T=" << ci.change_times[i].t << endl;
        for (auto const &[id, val] : ci.change_times[i].changes)
            if (ci.ID[id].scope == "TOP")
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
