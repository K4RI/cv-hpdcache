#ifndef DEF_SCOPE
#define DEF_SCOPE

#include <iostream>
#include <vector>

using namespace std;

class Scope
{
    public:
        Scope(string name = "", Scope* parent = NULL);
        Scope* insert(string name);
        Scope* get_parent () const;
        string get_name () const;

    private:
        string m_name;
        Scope* m_parent;
        vector<Scope*> m_children;

    friend ostream& operator<<( ostream &os, Scope const& scope );
};

#endif
