#ifndef INTERPRETEUR_H
#define INTERPRETEUR_H

#include "Symbole.h"
#include "Lecteur.h"
#include "Exceptions.h"
#include "TableSymboles.h"
#include "ArbreAbstrait.h"

class Interpreteur {
public:
    Interpreteur(ifstream & fichier); // Construit un interpréteur pour interpreter
    //  le programme dans  fichier 

    void analyse(); // Si le contenu du fichier est conforme à la grammaire,
    //   cette méthode se termine normalement et affiche un message "Syntaxe correcte".
    //   la table des symboles (ts) et l'arbre abstrait (arbre) auront été construits
    // Sinon, une exception sera levée

    inline const TableSymboles & getTable() const {
        return m_table;
    } // accesseur	

    inline Noeud* getArbre() const {
        return m_arbre;
    } // accesseur

    bool getSynthaxeIncorrecte() const {
        return synthaxeIncorrecte;
    }

    bool setSynthaxeIncorrecte() {
        this->synthaxeIncorrecte = true;
    }

private:
    bool synthaxeIncorrecte = false;

    Lecteur m_lecteur; // Le lecteur de symboles utilisé pour analyser le fichier -> tête de lecture du programme (fichier texte)
    TableSymboles m_table; // La table des symboles valués -> contient la liste des variables tout au long du programme
    Noeud* m_arbre; // L'arbre abstrait -> arbres des instructions (construit pendant l'execution du programme)

    // Implémentation de la grammaire
    Noeud* programme(); //   <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    Noeud* seqInst(); //     <seqInst> ::= <inst> { <inst> }
    Noeud* inst(); //        <inst> ::= <affectation> ; | <instSiRiche> | <instTantQue> | <instRepeter> | <instPour | <instEcrire> | <instLire> ; 
    Noeud* affectation(); // <affectation> ::= <variable> = <expression> 
    Noeud* expression(); //  <expression> ::= <facteur> { <opBinaire> <facteur> }
    Noeud* facteur(); //     <facteur> ::= <entier>  |  <variable>  |  - <facteur>  | non <facteur> | ( <expression> )
    //   <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
    Noeud* chaine(); //  <chaine> ::= " * "

    Noeud* instSiRiche(int i = 0, int j = 0, bool b = false); //  <instSiRiche> ::= si (<expression>) <seqInst> { sinonsi (<expression>) <seqInst> } [sinon <seqInst>] finsi
    Noeud* instTantQue(int i = 0, bool b = false); //  <instTantQue> ::= tantque (<expression>) <seqInst> fintantque
    Noeud* instRepeter(int i = 0, bool b = false); //  <instRepeter> ::= repeter <seqInst> jusqua (<expression>)
    Noeud* instPour(int i = 0, bool b = false, bool p = false); //  <instPour> ::= pour ( [ <affectation> ] : <expression> ; [ <affectation> ] ) <seqInst> finpour
    Noeud* instEcrire(int i = 0, int j = 0, int k = 0, bool b = false); //  <instEcrire> ::= ecrire ( <expression> | <chaine> { , <expression> | <chaine> } )
    Noeud* instLire(int i = 0, int j = 0, bool b = false); //  <instLire> ::= lire ( <variable> { , <variable> } )


    // outils pour simplifier l'analyse syntaxique
    void tester(const string & symboleAttendu) const throw (SyntaxeException); // Si symbole courant != symboleAttendu, on lève une exception
    void testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException); // Si symbole courant != symboleAttendu, on lève une exception, sinon on avance
    void erreur(const string & mess) const throw (SyntaxeException); // Lève une exception "contenant" le message mess
};

#endif /* INTERPRETEUR_H */
