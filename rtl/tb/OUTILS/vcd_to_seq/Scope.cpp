/***
 * Structure d'arbre pour hiérarchiser les scope d'une trace VCD.
 * 
***/

#include "Scope.hpp"

using namespace std;

Scope::Scope(string name, Scope* parent) : m_name(name), m_parent(parent), m_children({})
{
}

// Déclare un nouveau noeud fils et renvoie un pointeur vers lui.
Scope* Scope::insert(string name)
{
    Scope* child = new Scope(name, this);
    m_children.push_back(child);
    return child;
}

Scope* Scope::get_parent() const
{
    return m_parent;
}

string Scope::get_name() const
{
    return m_name;
}

// Affiche en flux.
ostream& operator<<( ostream &os, Scope const& scope )
{
    os << scope.get_name();
    
    if (scope.m_children.size() != 0) {
        os << " (";    
        for (Scope* const s : scope.m_children)
            os << *s << " ";
        os << ")";
    }
    return os;
}
