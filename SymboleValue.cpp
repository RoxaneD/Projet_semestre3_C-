#include "SymboleValue.h"
#include "Exceptions.h"
#include <stdlib.h>

SymboleValue::SymboleValue(const Symbole & s) :
Symbole(s.getChaine()) {
  if (s == "<ENTIER>") {
    m_valeur = atoi(s.getChaine().c_str()); // c_str convertit une string en char*
    m_defini = true;
  } else {
    m_defini = false;
  }
}

int SymboleValue::executer() {
  if (!m_defini) throw IndefiniException(); // on lève une exception si valeur non définie
  return m_valeur;
}

void SymboleValue::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    if (this->getChaine() != "") {
        cout << setw(4 * indentation) << "" << this->getChaine();
    } else {
        cout << setw(4 * indentation) << "" << m_valeur;
    }
}

ostream & operator<<(ostream & cout, const SymboleValue & symbole) {
  cout << "\t" << (Symbole) symbole << "\t\t - Valeur=";
  if (symbole.m_defini) cout << symbole.m_valeur << " ";
  else cout << "indefinie ";
  return cout;
}
