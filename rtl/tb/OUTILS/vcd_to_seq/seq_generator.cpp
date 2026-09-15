
#include <unordered_map>
#include "seq_generator.hpp"


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
    string clk_id = "";
    for (auto const &[gate, output, input, datas] : v) {
        inputvals[input] = "";
        outputvals[output] = "";
        for (data_gate const & d : datas) {
            datavals[d.name] = "";
        }
        if (gate == "CLK") {
            for (auto const &[k, v] : ID) {
                if (scopesig(v) == input) {
                    clk_id = k;
                    break;
                }
            }
        }
    }
    if (clk_id == "")
        throw string("pas trouvé de CLK dans le fichier .gates");

    string scopesig_s;
    for(int i(0); i<changes.size(); ++i) {

        // si l'id de la CLK donne une val 1 : @(posedge clk)
        if (changes[i].changes[clk_id] == "1" || i==0) {
            lines.push_back("-- t=" + to_string(changes[i].t));
            lines.push_back("\"CLK\"");
            
            // mettre à jour les valeurs
            for (auto const &[id, val] : changes[i].changes) {
                scopesig_s = scopesig(ID[id]);
                
                if (inputvals.contains(scopesig_s)) {
                    inputvals[scopesig_s] = val;
                } else if (outputvals.contains(scopesig_s)) {
                    outputvals[scopesig_s] = val;
                } else if (datavals.contains(scopesig_s)) {
                    datavals[scopesig_s] = toDec(val);
                }
            }

            // repérer les handshakes
            for (auto &[gate, output, input, datas] : v) {
                if (outputvals[output] == "1" & (inputvals[input] == "1" || input == "")) {
                    string line_action = "\"" + gate;
                    for (data_gate & d : datas) {
                        string val = datavals[d.name];
                        if (d.abstraction.size() > 0) {
                            val = (d.abstraction)[datavals[d.name]];
                        }
                        line_action += (" !" + val);
                    }
                    line_action += "\"";
                    lines.push_back(line_action);
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
