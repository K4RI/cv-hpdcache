
#include "vcd_parser.hpp"
#include "seq_generator.hpp"
#include <cstring>


string removeFileExtension(string filename) {
    size_t pos = filename.rfind('.');
    if (pos != string::npos) {
        return filename.substr(0, pos);
    } else {
        return filename;
    }
}

int main(int argc, char **argv) {
    try {
        string filename = argv[1];

        bool verbose = (strcmp (argv[2], "1") == 0);

        map_ids ID;
        vec_changes changes;

        // lire le VCD
        parseVCDFile(filename, ID, changes, verbose);
        if (verbose) {
            cout << ID << endl;
            cout << changes_id({changes, ID}) << endl;
        }

        string name = removeFileExtension(filename); // retirer le .vcd
        vec_gates v = parseGATESFile("mau", verbose);
        generateSEQFile(name, ID, changes, v, verbose); // écrire dans name.seq

        return EXIT_SUCCESS;
    }
    catch (string& s) {
        std::cout << "ERREUR: \"" << s << "\"" << endl;
    }
}
