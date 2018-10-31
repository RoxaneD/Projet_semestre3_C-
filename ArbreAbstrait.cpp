#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"
#include <typeinfo>

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        m_instructions[i]->traduitEnCPP(cout, indentation);
        cout << endl;
    }
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
    if (instruction != nullptr) m_instructions.push_back(instruction);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
    int valeur = m_expression->executer(); // On exécute (évalue) l'expression
    ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudAffectation::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    m_variable->traduitEnCPP(cout,indentation);
    cout << " = ";
    m_expression->traduitEnCPP(cout,0);
    cout << ";";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudChaine
////////////////////////////////////////////////////////////////////////////////

NoeudChaine::NoeudChaine(Symbole s)
: m_contenu(s) {
}

int NoeudChaine::executer() {
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudChaine::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << m_contenu.getChaine();
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
    int og, od, valeur;
    if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
    if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
    // Et on combine les deux opérandes en fonctions de l'opérateur
    if (this->m_operateur == "+") valeur = (og + od);
    else if (this->m_operateur == "-") valeur = (og - od);
    else if (this->m_operateur == "*") valeur = (og * od);
    else if (this->m_operateur == "==") valeur = (og == od);
    else if (this->m_operateur == "!=") valeur = (og != od);
    else if (this->m_operateur == "<") valeur = (og < od);
    else if (this->m_operateur == ">") valeur = (og > od);
    else if (this->m_operateur == "<=") valeur = (og <= od);
    else if (this->m_operateur == ">=") valeur = (og >= od);
    else if (this->m_operateur == "et") valeur = (og && od);
    else if (this->m_operateur == "ou") valeur = (og || od);
    else if (this->m_operateur == "non") valeur = (!og);
    else if (this->m_operateur == "/") {
        if (od == 0) throw DivParZeroException();
        valeur = og / od;
    }
    return valeur; // On retourne la valeur calculée
}

void NoeudOperateurBinaire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    m_operandeGauche->traduitEnCPP(cout, indentation);
    cout << " " << m_operateur.getChaine() << " ";
    m_operandeDroit->traduitEnCPP(cout,indentation);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstSi::executer() {
    if (m_condition->executer()) m_sequence->executer();
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstSi::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4 * indentation) << "" << "if (";
    m_condition->traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequence->traduitEnCPP(cout, indentation + 1);
    cout << setw(4 * indentation) << "" << "}" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSiRiche
////////////////////////////////////////////////////////////////////////////////

NoeudInstSiRiche::NoeudInstSiRiche()
: m_conditions(), m_sequences() {
}

int NoeudInstSiRiche::executer() {
    int i = 0;
    bool b = false;
    if (m_conditions[i]->executer()) {
        m_sequences[i]->executer();
        b = true;
    }
    i = 1;
    while (m_conditions[i] != NULL && m_sequences[i] != NULL && b == false) {
        if (m_conditions[i]->executer()) {
            m_sequences[i]->executer();
            b = true;
        }
        i++;
    }
    if (m_sequences[i] != NULL && b == false) {
        m_sequences[i]->executer();
    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstSiRiche::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    int i = 0;
    cout << setw(4 * indentation) << "" << "if (";
    m_conditions[i]->traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequences[i]->traduitEnCPP(cout, indentation + 1);
    i = 1;
    while (m_conditions[i] != NULL && m_sequences[i] != NULL) {
        cout << setw(4 * indentation) << "" << "} else if (";
        m_conditions[i]->traduitEnCPP(cout, 0);
        cout << ") {" << endl;
        m_sequences[i]->traduitEnCPP(cout, indentation + 1);
        i++;
    }
    if (m_sequences[i] != NULL) {
        cout << setw(4 * indentation) << "" << "} else {" << endl;
        m_sequences[i]->traduitEnCPP(cout, indentation + 1);
    }
    cout << setw(4 * indentation) << "" << "}";
}

void NoeudInstSiRiche::ajouteCon(Noeud* condition) {
    if (condition != nullptr) m_conditions.push_back(condition);
}

void NoeudInstSiRiche::ajouteSeq(Noeud* sequence) {
    if (sequence != nullptr) m_sequences.push_back(sequence);
}

// NoeudTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {
    while (m_condition->executer()) {
        m_sequence->executer();
    }
    return 0; // valeur ne représente rien
}

void NoeudInstTantQue::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4 * indentation) << "" << "while (";
    m_condition->traduitEnCPP(cout,0);
    cout << ") {" << endl;
    m_sequence->traduitEnCPP(cout, indentation+1);
    cout << setw(4 * indentation) << "" << "}";
}

// NoeudInstRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudInstRepeter::NoeudInstRepeter(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstRepeter::executer() {
    m_sequence->executer();
    while (!m_condition->executer()) {
        m_sequence->executer();
    }
    return 0; // valeur ne représente rien
}

void NoeudInstRepeter::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4 * indentation) << "" << "do {" << endl;
    m_sequence->traduitEnCPP(cout,indentation+1);
    cout << setw(4 * indentation) << "" << "} while (";
    m_condition->traduitEnCPP(cout, 0);
    cout << ");";
}

// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud* affect1, Noeud* condition, Noeud* affect2, Noeud* sequence)
: m_affect1(affect1), m_condition(condition), m_affect2(affect2), m_sequence(sequence) {
}

int NoeudInstPour::executer() {
    if (m_affect1 != nullptr && m_affect2 != nullptr) {
        for (m_affect1->executer(); m_condition->executer(); m_affect2->executer()) {
            m_sequence->executer();
        }
    } else if (m_affect1 != nullptr && m_affect2 == nullptr) {
        m_affect1->executer();
        while (m_condition->executer()) {
            m_sequence->executer();
        }
    } else if (m_affect1 == nullptr && m_affect2 != nullptr) {
        while (m_condition->executer()) {
            m_sequence->executer();
            m_affect2->executer();
        }
    } else {
        while (m_condition->executer()) {
            m_sequence->executer();
        }
    }
    return 0; // valeur ne représente rien ici
}

void NoeudInstPour::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    // on code juste le cas où les trois conditions sont remplies
    if (m_affect1 != nullptr && m_affect2 != nullptr) {
        cout << setw(4 * indentation) << "" << "for (";
        m_affect1->traduitEnCPP(cout,0);
        cout << " ";
        m_condition->traduitEnCPP(cout,0); cout << "; ";
        m_affect2->traduitEnCPP(cout,0);
        cout << ") {" << endl;
        m_sequence->traduitEnCPP(cout,indentation+1);
        cout << setw(4 * indentation) << "" << "}";
    }
}

// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////

NoeudInstEcrire::NoeudInstEcrire() : m_contenuAEcrire() {
}

int NoeudInstEcrire::executer() {
    for (unsigned int i = 0; i < m_contenuAEcrire.size(); i++) {
        if (*((SymboleValue*) m_contenuAEcrire[i]) == "<CHAINE>") {
            string aAfficher = m_contenuAEcrire[i]->getChaine();
            aAfficher.erase(0, 1);
            aAfficher.pop_back();
            cout << aAfficher;
        } else {
            cout << m_contenuAEcrire[i]->executer();
        }
    }
    cout << endl;
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstEcrire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    for (unsigned int i = 0; i < m_contenuAEcrire.size(); i++) {
        if (*((SymboleValue*) m_contenuAEcrire[i]) == "<CHAINE>") {
            string aAfficher = m_contenuAEcrire[i]->getChaine();
            aAfficher.erase(0, 1);
            aAfficher.pop_back();
            cout << "cout << " << aAfficher << ";" << endl;
        } else {
            m_contenuAEcrire[i]->traduitEnCPP(cout,0);
        }
    }
    cout << endl;
}

void NoeudInstEcrire::ajoute(SymboleValue* instruction) {
    if (instruction != nullptr) m_contenuAEcrire.push_back(instruction);
}

// NoeudInstLire
////////////////////////////////////////////////////////////////////////////////

NoeudInstLire::NoeudInstLire() : m_contenuALire() {
}

int NoeudInstLire::executer() {
    int valeur;
    for (unsigned int i = 0; i < m_contenuALire.size(); i++) {
        cout << "Valeur de : " << m_contenuALire[i]->getChaine() << endl;
        cin >> valeur;
        m_contenuALire[i]->setValeur(valeur);
    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstLire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    int valeur;
    for (unsigned int i = 0; i < m_contenuALire.size(); i++) {
        cout << "Valeur de : " << m_contenuALire[i]->getChaine() << endl;
        cin >> valeur;
        m_contenuALire[i]->setValeur(valeur);
    }
}

void NoeudInstLire::ajoute(SymboleValue* contenu) {
    if (contenu != nullptr) m_contenuALire.push_back(contenu);
}