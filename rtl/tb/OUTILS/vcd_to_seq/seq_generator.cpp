
#include <unordered_map>
#include "seq_generator.hpp"

// Change '0' en "1B0", et '1' en "1B1". Utilisé dans toArr.
// Vraiment utile ?
string bin2onebit(char c) {
    if (c == '0') {
        return "1B0";
    } else if (c == '1') {
        return "1B1";
    } else {
        throw string("erreur bin2onebit : ") + c;
    }
}

// Si tableau binaire : le renvoie sous forme d'array LNT.
string toArr(string v, string arr_type_name) {
    if (v[0] == 'b') { // "ARR (1B0, 1B1...)"
        string o = arr_type_name + " (";
        for(int i(1); i<v.length(); ++i) {
            o = o + bin2onebit(v[i]) + ", ";
        }
        return o.substr(0, o.length() - 2) + ')';
    };
    assert (v.length() == 1);
    return bin2onebit(v[0]);
}

// Si tableau binaire : le renvoie sous forme décimale.
string toDec(string v) {
    if (v[0] == 'b') {
        int n = 0;
        int b = 1;
        for(int i(v.length()-1); i>0; --i) {
            n += b* (v[i] - '0');
            b *= 2;
        }
        return to_string(n);
    };
    assert (v.length() == 1); // juste 0 ou 1
    return to_string(v[0] - '0');
}


ostream& operator<<(ostream &os, vec_gates v){
    for(int i(0); i<v.size(); ++i)
        os << v[i].gate << " : output=" << v[i].output << ", input=" << v[i].input << ", data=" << v[i].data << endl;
    return os;
}

vec_gates parseGATESFile(string filename, bool verbose){
    ifstream file (filename + ".gates");
    
    if (file.is_open())
    {
        string line;
        vec_gates v;
        gate_match current;

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
                        current.data = s;
                        break;
                    
                    default:
                        cout << "EUH" << endl;
                        break;
                }
                
            }
        }
        if (current.gate + current.output + current.input + current.data != "") { // last gate
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


void generateSEQFile(string filename, map_ids ID, vec_changes changes, vec_gates v, bool verbose)
{

    if (verbose) {
        cout << changes.size() << " changements" << endl;
    }    

    vector<string> lines;

    unordered_map<string, string> inputvals;
    unordered_map<string, string> outputvals;
    unordered_map<string, string> datavals;

    // remplir les maps signal->valeur, et identifier l'id de l'horloge
    char clk_id = ' ';
    for (auto const &[gate, output, input, data] : v) {
        inputvals[input] = "";
        outputvals[output] = "";
        datavals[data] = "";
        if (gate == "CLK") {
            for (auto const &[k, v] : ID) {
                if (v.reference == input) {
                    clk_id = k;
                    break;
                }
            }
        }
    }
    if (clk_id == ' ')
        throw string("pas trouvé de CLK dans le fichier .gates");

    for(int i(0); i<changes.size(); ++i) {

        // si l'id de la CLK donne une val 1 : @(posedge clk)
        if (changes[i].changes[clk_id] == "1") {
            lines.push_back("\"CLK\"");
            
            for (auto const &[id, val] : changes[i].changes) {
                // if (ID[id].scope == "TOP") {
                
                if (inputvals.contains(ID[id].reference)) {
                    inputvals[ID[id].reference] = val;
                } else if (outputvals.contains(ID[id].reference)) {
                    outputvals[ID[id].reference] = val;
                } else if (datavals.contains(ID[id].reference)) {
                    datavals[ID[id].reference] = toDec(val);
                }
            }

            for (auto const &[gate, output, input, data] : v) {
                if (inputvals[input] == "1" & outputvals[output] == "1") {
                    lines.push_back("-- t=" + to_string(changes[i].t));
                    lines.push_back("\"" + gate + " !" + datavals[data] + "\"");
                }
            }
        }
    }

    // retirer tous les "CLK" en fin de trace
    for (int i = lines.size()-1; i >= 0; --i) {
        if (lines[i] == "\"CLK\"") {
            lines.pop_back();
        } else {
            break;
        }
    }

    ofstream file (filename + ".seq");
    if(file.is_open())
    {
        for (string line : lines) {
            file << line << endl;
        }
        file.close();
    }
    else
    {
        throw string("erreur ouverture fichier écriture : ") + filename;
    }
}
