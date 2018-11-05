#ifndef ARBREABSTRAIT_H
#define ARBREABSTRAIT_H

// Contient toutes les déclarations de classes nécessaires
//  pour représenter l'arbre abstrait

#include <vector>
#include <iostream>
#include <iomanip>
using namespace std;

#include "Symbole.h"
#include "Exceptions.h"
class SymboleValue;

////////////////////////////////////////////////////////////////////////////////

class Noeud {
    // Classe abstraite dont dériveront toutes les classes servant à représenter l'arbre abstrait
    // Remarque : la classe ne contient aucun constructeur
public:
    virtual int executer() = 0; // Méthode pure (non implémentée) qui rend la classe abstraite

    virtual void ajoute(Noeud* instruction) {
        throw OperationInterditeException();
    }

    virtual void traduitEnCPP(ostream & cout, unsigned int indentation) const {
        cout << this;
    }

    virtual void traduitEnCPPAffectionSans(ostream & cout, unsigned int indentation) const {
        cout << "Opération impossible. Possible seulement pour une affectation.";
    }

    virtual ~Noeud() {
    } // Présence d'un destructeur virtuel conseillée dans les classes abstraites

};

////////////////////////////////////////////////////////////////////////////////

class NoeudSeqInst : public Noeud {
    // Classe pour représenter un noeud "sequence d'instruction"
    //  qui a autant de fils que d'instructions dans la séquence
public:
    NoeudSeqInst(); // Construit une séquence d'instruction vide

    ~NoeudSeqInst() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute chaque instruction de la séquence
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;
    void ajoute(Noeud* instruction); // Ajoute une instruction à la séquence

private:
    vector<Noeud *> m_instructions; // pour stocker les instructions de la séquence
};

////////////////////////////////////////////////////////////////////////////////

class NoeudAffectation : public Noeud {
    // Classe pour représenter un noeud "affectation"
    //  composé de 2 fils : la variable et l'expression qu'on lui affecte
public:
    NoeudAffectation(Noeud* variable, Noeud* expression); // construit une affectation

    ~NoeudAffectation() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute (évalue) l'expression et affecte sa valeur à la variable
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;
    void traduitEnCPPAffectionSans(ostream & cout, unsigned int indentation) const;
    
private:
    Noeud* m_variable;
    Noeud* m_expression;
};

////////////////////////////////////////////////////////////////////////////////

class NoeudChaine : public Noeud {
    // Classe pour représenter un noeud "chaine"
public:
    NoeudChaine(Symbole s);
    // Construit une "chaine" avec son contenu (Symbole)

    ~NoeudChaine() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute (évalue) l'expression
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

private:
    Symbole m_contenu;
};

////////////////////////////////////////////////////////////////////////////////

class NoeudOperateurBinaire : public Noeud {
    // Classe pour représenter un noeud "opération binaire" composé d'un opérateur
    //  et de 2 fils : l'opérande gauche et l'opérande droit
public:
    NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit);
    // Construit une opération binaire : operandeGauche operateur OperandeDroit

    ~NoeudOperateurBinaire() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute (évalue) l'opération binaire)
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

private:
    Symbole m_operateur;
    Noeud* m_operandeGauche;
    Noeud* m_operandeDroit;
};

////////////////////////////////////////////////////////////////////////////////

class NoeudInstSi : public Noeud {
    // Classe pour représenter un noeud "instruction si"
    //  et ses 2 fils : la condition du si et la séquence d'instruction associée
public:
    NoeudInstSi(Noeud* condition, Noeud* sequence);
    // Construit une "instruction si" avec sa condition et sa séquence d'instruction

    ~NoeudInstSi() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute l'instruction si : si condition vraie on exécute la séquence
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

private:
    Noeud* m_condition;
    Noeud* m_sequence;
};

////////////////////////////////////////////////////////////////////////////////

class NoeudInstSiRiche : public Noeud {
    // Classe pour représenter un noeud "instruction siRiche"
    //  et sa séquence d'instructions associée
public:
    NoeudInstSiRiche(); // Construit une séquence d'instructions vide

    ~NoeudInstSiRiche() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute l'instruction siRiche : si condition vraie on exécute la séquence
    // sinon -> regarder la condition suivante (etc ...)
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;
    void ajouteCon(Noeud* condition);
    void ajouteSeq(Noeud* sequence);

private:
    vector<Noeud *> m_conditions;
    vector<Noeud *> m_sequences;
};

////////////////////////////////////////////////////////////////////////////////

class NoeudInstTantQue : public Noeud {
    // Classe pour représenter un noeud "instruction tantQue"
    //  et sa séquence d'instructions associée
public:
    NoeudInstTantQue(Noeud* condition, Noeud* sequence);

    ~NoeudInstTantQue() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute l'instruction tantQue : si condition vraie on exécute la séquence (peut etre pas de passage)
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

private:
    Noeud* m_condition;
    Noeud* m_sequence;
};

////////////////////////////////////////////////////////////////////////////////

class NoeudInstRepeter : public Noeud {
    // Classe pour représenter un noeud "instruction repeter"
    //  et sa séquence d'instructions associée
public:
    NoeudInstRepeter(Noeud* condition, Noeud* sequence);

    ~NoeudInstRepeter() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute l'instruction repeter : executer l'instruction jusqu'à ce que la condition soit vraie (forcément un passage)
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

private:
    Noeud* m_condition;
    Noeud* m_sequence;
};

////////////////////////////////////////////////////////////////////////////////

class NoeudInstPour : public Noeud {
    // Classe pour représenter un noeud "instruction pour"
    //  et sa séquence d'instructions associée
public:
    NoeudInstPour(Noeud* affect1, Noeud* condition, Noeud* affect2, Noeud* sequence);

    ~NoeudInstPour() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute l'instruction : executer un for
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

private:
    Noeud* m_affect1;
    Noeud* m_condition;
    Noeud* m_affect2;
    Noeud* m_sequence;
};

////////////////////////////////////////////////////////////////////////////////

class NoeudInstEcrire : public Noeud {
    // Classe pour représenter un noeud "ecrire"
    //  qui a autant de fils que d'instructions dans la séquence
public:
    NoeudInstEcrire(); // Construit une séquence de contenu vide

    ~NoeudInstEcrire() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute chaque instruction de la séquence
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;
    void ajoute(SymboleValue* instruction); // Ajoute une instruction à la séquence

private:
    vector<SymboleValue*> m_contenuAEcrire; // pour stocker les instructions de la séquence
};

////////////////////////////////////////////////////////////////////////////////

class NoeudInstLire : public Noeud {
    // Classe pour représenter un noeud "lire"
public:
    NoeudInstLire(); // Construit une séquence de contenu vide

    ~NoeudInstLire() {
    } // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute chaque instruction de la séquence
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;
    void ajoute(SymboleValue* contenu); // Ajoute du contenu à la séquence

private:
    vector<SymboleValue*> m_contenuALire;
};

#endif /* ARBREABSTRAIT_H */


