#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr) {
}

void Interpreteur::analyse() {
    m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const throw (SyntaxeException) {
    // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
    static char messageWhat[256];
    if (m_lecteur.getSymbole() != symboleAttendu) {
        sprintf(messageWhat,
                "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
                m_lecteur.getLigne(), m_lecteur.getColonne(),
                symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
        throw SyntaxeException(messageWhat);
    }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException) {
    // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
    tester(symboleAttendu);
    m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const throw (SyntaxeException) {
    // Lève une exception contenant le message et le symbole courant trouvé
    // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
    static char messageWhat[256];
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
    // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    testerEtAvancer("procedure");
    testerEtAvancer("principale");
    testerEtAvancer("(");
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    testerEtAvancer("finproc");
    tester("<FINDEFICHIER>");
    return sequence;
}

Noeud* Interpreteur::seqInst() {
    // <seqInst> ::= <inst> { <inst> }
    NoeudSeqInst* sequence = new NoeudSeqInst();
    try {
        do {
            sequence->ajoute(inst());
        } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "si"
                || m_lecteur.getSymbole() == "tantque" || m_lecteur.getSymbole() == "repeter"
                || m_lecteur.getSymbole() == "pour" || m_lecteur.getSymbole() == "ecrire"
                || m_lecteur.getSymbole() == "lire");
        // Tant que le symbole courant est un début possible d'instruction...
        // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction

        return sequence;
    } catch (SyntaxeException & e) {
        cout << e.what() << endl;
        return nullptr;
    }
}

Noeud* Interpreteur::inst() {
    //<inst> ::= <affectation> ; | <instSiRiche> | <instTantQue> 
    // | <instRepeter> | <instPour | <instEcrire> | <instLire> ; 
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        Noeud *affect = affectation();
        testerEtAvancer(";");
        return affect;
    } else if (m_lecteur.getSymbole() == "si") {
        return instSiRiche();
        // Compléter les alternatives chaque fois qu'on rajoute une nouvelle instruction
    } else if (m_lecteur.getSymbole() == "tantque") {
        return instTantQue();
    } else if (m_lecteur.getSymbole() == "repeter") {
        return instRepeter();
    } else if (m_lecteur.getSymbole() == "pour") {
        return instPour();
    } else if (m_lecteur.getSymbole() == "ecrire") {
        return instEcrire();
    } else if (m_lecteur.getSymbole() == "lire") {
        return instLire();
    } else {
        erreur("Instruction incorrecte");
    }
}

Noeud* Interpreteur::affectation() {
    // <affectation> ::= <variable> = <expression> 
    try {
        tester("<VARIABLE>");
        Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table et on la mémorise
        m_lecteur.avancer();
        testerEtAvancer("=");
        Noeud* exp = expression(); // On mémorise l'expression trouvée
        return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
    } catch (SyntaxeException & e) {
        cout << e.what() << endl;
        return nullptr;
    }
}

Noeud* Interpreteur::expression() {
    // <expression> ::= <facteur> { <opBinaire> <facteur> }
    //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
    try {
        Noeud* fact = facteur();
        while (m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-" ||
                m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/" ||
                m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == "<=" ||
                m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=" ||
                m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
                m_lecteur.getSymbole() == "et" || m_lecteur.getSymbole() == "ou") {
            Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
            m_lecteur.avancer();
            Noeud* factDroit = facteur(); // On mémorise l'opérande droit
            fact = new NoeudOperateurBinaire(operateur, fact, factDroit); // Et on construit un noeud opérateur binaire
        }
        return fact; // On renvoie fact qui pointe sur la racine de l'expression
    } catch (SyntaxeException & e) {
        cout << e.what() << endl;
        return nullptr;
    }
}

Noeud* Interpreteur::facteur() {
    // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
    Noeud* fact = nullptr;
    if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
        fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
        m_lecteur.avancer();
    } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
    } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("non"), facteur(), nullptr);
    } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
        m_lecteur.avancer();
        fact = expression();
        testerEtAvancer(")");
    } else {
        erreur("Facteur incorrect");
    }
    return fact;
}

Noeud * Interpreteur::chaine() {
    return new NoeudChaine(m_lecteur.getSymbole()); //censé retourner un NoeudChaine (Symbole de type CHAINE)
}

Noeud * Interpreteur::instSiRiche(int i, int j, bool b) { // OK (Erreur)
    //  <instSiRiche> ::= si (<expression>) <seqInst> { sinonsi (<expression>) <seqInst> } [sinon <seqInst>] finsi
    NoeudInstSiRiche* sequence = new NoeudInstSiRiche();
    Noeud* exp;
    try {
        if (i == 0) {
            testerEtAvancer("si");
            i = 1;
        }
        if (i == 1) {
            testerEtAvancer("(");
            i = 2;
        }
        if (i == 2) {
            exp = expression();
            if (exp != nullptr) {
                sequence->ajouteCon(exp); // On mémorise la condition
            } else {
                throw SyntaxeException();
            }
            i = 3;
        }
        if (i == 3) {
            testerEtAvancer(")");
            i = 4;
        }
        if (i == 4) {
            sequence->ajouteSeq(seqInst()); // On mémorise la séquence d'instruction
            i = 5;
        }
        if (i == 5) {
            while (m_lecteur.getSymbole() == "sinonsi" || j != 0) {
                if (j == 0) {
                    m_lecteur.avancer();
                    j = 1;
                }
                if (j == 1) {
                    testerEtAvancer("(");
                    j = 2;
                }
                if (j == 2) {
                    exp = expression();
                    if (exp != nullptr) {
                        sequence->ajouteCon(exp); // On mémorise la condition
                    } else {
                        throw SyntaxeException();
                    }
                    j = 3;
                }
                if (j == 3) {
                    testerEtAvancer(")");
                    j = 4;
                }
                if (j == 4) {
                    sequence->ajouteSeq(seqInst());
                    j = 0;
                }
            }
            i = 6;
        }
        if (i == 6) {
            if (m_lecteur.getSymbole() == "sinon") {
                if (j == 0) {
                    m_lecteur.avancer();
                    j = 1;
                }
                if (j == 1) {
                    exp = seqInst();
                    if (exp != nullptr) {
                        sequence->ajouteSeq(exp); // On mémorise la condition
                    } else {
                        throw SyntaxeException();
                    }
                    j = 0;
                }
            }
            i = 7;
        }
        if (i == 7) {
            testerEtAvancer("finsi");
            i = 8;
        }
        if (i == 8 && b == false) {
            return sequence; // Renvoie un noeud Instruction SiRiche 
        } else if (i == 8) {
            m_arbre = nullptr;
            return nullptr;
        }
    } catch (SyntaxeException & e) {
        if (i == 2) {
            ;
        } else if (i == 5 && j == 2) {
            ;
        } else if (i == 6 && j == 1) {
            ;
        } else {
            cout << e.what() << endl;
        }
        bool b = true;
        setSynthaxeIncorrecte();
        if (j != 0) {
            j = j + 1;
        } else {
            i = i + 1;
        }
        instSiRiche(i, j, b);
    }
}

Noeud * Interpreteur::instTantQue(int i, bool b) { // OK (Erreur)
    //  <instTantQue> ::= tantque (<expression>) <seqInst> fintantque
    Noeud* condition;
    Noeud* sequence;
    try {
        if (i == 0) {
            testerEtAvancer("tantque");
            i = 1;
        }
        if (i == 1) {
            testerEtAvancer("(");
            i = 2;
        }
        if (i == 2) {
            condition = expression();
            if (condition == nullptr) {
                throw SyntaxeException();
            }
            i = 3;
        }
        if (i == 3) {
            testerEtAvancer(")");
            i = 4;
        }
        if (i == 4) {
            sequence = seqInst();
            if (sequence == nullptr) {
                throw SyntaxeException();
            }
            i = 5;
        }
        if (i == 5) {
            testerEtAvancer("fintantque");
            i = 6;
        }
        if (i == 6 && b == false) {
            return new NoeudInstTantQue(condition, sequence);
        } else if (i == 6) {
            m_arbre = nullptr;
            return nullptr;
        }
    } catch (SyntaxeException & e) {
        if (i == 2 || i == 4) {
            ;
        } else {
            cout << e.what() << endl;
        }
        bool b = true;
        setSynthaxeIncorrecte();
        i = i + 1;
        instTantQue(i, b);
    }
}

Noeud * Interpreteur::instRepeter(int i, bool b) { // OK (Erreur)
    //  repeter <seqInst> jusqua (<expression>)
    Noeud* sequence;
    Noeud* condition;
    try {
        if (i == 0) {
            testerEtAvancer("repeter");
            i = 1;
        }
        if (i == 1) {
            sequence = seqInst();
            if (sequence == nullptr) {
                throw SyntaxeException();
            }
            i = 2;
        }
        if (i == 2) {
            testerEtAvancer("jusqua");
            i = 3;
        }
        if (i == 3) {
            testerEtAvancer("(");
            i = 4;
        }
        if (i == 4) {
            condition = expression();
            if (condition == nullptr) {
                throw SyntaxeException();
            }
            i = 5;
        }
        if (i == 5) {
            testerEtAvancer(")");
            i = 6;
        }
        if (i == 6 && b == false) {
            return new NoeudInstRepeter(condition, sequence);
        } else if (i == 6) {
            m_arbre = nullptr;
            return nullptr;
        }
    } catch (SyntaxeException & e) {
        if (i == 1 || i == 4) {
            ;
        } else {
            cout << e.what() << endl;
        }
        bool b = true;
        setSynthaxeIncorrecte();
        i = i + 1;
        instRepeter(i, b);
    }
}

Noeud * Interpreteur::instPour(int i, bool b, bool p) { // OK (Erreur)
    //  pour ([<affectation>] ; <expression> ; [<affectation>]) <seqInst> finpour
    Noeud* affect1;
    Noeud* condition;
    Noeud* affect2;
    Noeud* sequence;
    try {
        if (i == 0) {
            testerEtAvancer("pour");
            i = 1;
        }
        if (i == 1) {
            testerEtAvancer("(");
            i = 2;
        }
        if (i == 2) {
            affect1 = nullptr;
            if (m_lecteur.getSymbole() == "<VARIABLE>") {
                affect1 = affectation();
            }
            i = 3;
        }
        if (i == 3) {
            testerEtAvancer(";");
            i = 4;
        }
        if (i == 4) {
            condition = expression();
            if (condition == nullptr) {
                throw SyntaxeException();
            }
            i = 5;
        }
        if (i == 5) {
            testerEtAvancer(";");
            i = 6;
        }
        if (i == 6) {
            affect2 = nullptr;
            if (m_lecteur.getSymbole() == "<VARIABLE>") {
                affect2 = affectation();
            }
            i = 7;
        }
        if (i == 7) {
            testerEtAvancer(")");
            i = 8;
        }
        if (i == 8) {
            if (p == true) {
                m_lecteur.avancer();
            }
            sequence = seqInst();
            if (sequence == nullptr) {
                throw SyntaxeException();
            }
            i = 9;
        }
        if (i == 9) {
            testerEtAvancer("finpour");
            i = 10;
        }
        if (i == 10 && b == false) {
            return new NoeudInstPour(affect1, condition, affect2, sequence);
        } else if (i == 10) {
            m_arbre = nullptr;
            return nullptr;
        }
    } catch (SyntaxeException & e) {
        if (i == 7) {
            p = true;
        }
        if (i == 4 || i == 8) {
            ;
        } else {
            cout << e.what() << endl;
        }
        bool b = true;
        setSynthaxeIncorrecte();
        i = i + 1;
        instPour(i, b, p);
    }
}

Noeud * Interpreteur::instEcrire(int i, int j, int k, bool b) {
    //  ecrire ( <expression> | <chaine> { , <expression> | <chaine> } )
    NoeudInstEcrire* sequence;
    Noeud* chain;
    Noeud* condition;
    try {
        if (i == 0) {
            testerEtAvancer("ecrire");
            i = 1;
        }
        if (i == 1) {
            testerEtAvancer("(");
            i = 2;
        }
        if (i == 2) {
            sequence = new NoeudInstEcrire();
            if (sequence == nullptr) {
                throw SyntaxeException();
            }
            i = 3;
        }
        if (i == 3) {
            if (m_lecteur.getSymbole() == "<CHAINE>") {
                if (j == 0) {
                    chain = chaine();
                    if (chain == nullptr) {
                        throw SyntaxeException();
                    } else {
                        sequence->ajoute((SymboleValue*) chain);
                    }
                    j = 1;
                }
                if (j == 1) {
                    m_lecteur.avancer();
                    j = 0;
                }
            } else {
                condition = expression();
                if (condition == nullptr) {
                    throw SyntaxeException();
                } else {
                    sequence->ajoute((SymboleValue*) condition);
                }
            }
            i = 4;
        }
        if (i == 4) {
            while (m_lecteur.getSymbole() == ",") {
                if (j == 0) {
                    m_lecteur.avancer();
                    j = 1;
                }
                if (j == 1) {
                    if (m_lecteur.getSymbole() == "<CHAINE>") {
                        if (k == 0) {
                            chain = chaine();
                            if (chain == nullptr) {
                                throw SyntaxeException();
                            } else {
                                sequence->ajoute((SymboleValue*) chain);
                            }
                            k = 1;
                        }
                        if (k == 1) {
                            m_lecteur.avancer();
                            k = 0;
                        }

                    } else {
                        condition = expression();
                        if (condition == nullptr) {
                            throw SyntaxeException();
                        } else {
                            sequence->ajoute((SymboleValue*) condition);
                        }
                    }
                    j = 0;
                }
            }
            i = 5;
        }
        if (i == 5) {
            testerEtAvancer(")");
            i = 6;
        }
        if (i == 6 && b == false) {
            return sequence;
        } else if (i == 6) {
            m_arbre = nullptr;
            return nullptr;
        }
    } catch (SyntaxeException & e) {
        if (i == 2) {
            ;
        } else if (i == 3) {
            ;
        } else if (i == 4 && j == 1) {
            ;
        } else {
            cout << e.what() << endl;
        }
        bool b = true;
        setSynthaxeIncorrecte();
        if (k != 0) {
            k = k + 1;
        } else if (j != 0) {
            j = j + 1;
        } else {
            i = i + 1;
        }
        instEcrire(i, j, k, b);
    }
}

Noeud * Interpreteur::instLire(int i, int j, bool b) {
    //  lire (<variable> {,<variable>})
    cout << "i : " << i << " | j : " << j << endl;
    NoeudInstLire* sequence;
    SymboleValue* s;
    try {
        if (i == 0) {
            testerEtAvancer("lire");
            i = 1;
        }
        if (i == 1) {
            testerEtAvancer("(");
            i = 2;
        }
        if (i == 2) {
            sequence = new NoeudInstLire();
            if (sequence == nullptr) {
                throw SyntaxeException();
            }
            i = 3;
        }
        if (i == 3) {
            tester("<VARIABLE>");
            s = m_table.chercheAjoute(m_lecteur.getSymbole());
            if (s == nullptr) {
                throw SyntaxeException();
            } else {
                sequence->ajoute(s);
            }
            m_lecteur.avancer();
            i = 4;
        }
        if (i == 4) {
            while (m_lecteur.getSymbole() == ",") {
                if (j == 0) {
                    m_lecteur.avancer();
                    j = 1;
                }
                if (j == 1) {
                    tester("<VARIABLE>");
                    s = m_table.chercheAjoute(m_lecteur.getSymbole());
                    if (s == nullptr) {
                        throw SyntaxeException();
                    } else {
                        sequence->ajoute(s);
                    }
                    j = 2;
                }
                if (j == 2) {
                    m_lecteur.avancer();
                    j = 0;
                }
            }
            i = 5;
        }
        if (i == 5) {
            testerEtAvancer(")");
            i = 6;
        }
        if (i == 6 && b == false) {
            return sequence;
        } else if (i == 6) {
            m_arbre = nullptr;
            return nullptr;
        }
    } catch (SyntaxeException & e) {
        if (i == 4 && j == 1) {
            ;
        } else {
            cout << e.what() << endl;
        }
        bool b = true;
        setSynthaxeIncorrecte();
        if (j != 0) {
            j = j + 1;
        } else {
            i = i + 1;
        }
        instLire(i, j, b);
    }
}

void Interpreteur::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4 * indentation) << "" << "int main() {" << endl;
    int i = 1;
    while (i <= m_table.getTaille()) {
        int a = m_table[i-1].getValeur();
        stringstream ss;
        ss << a;
        string b = ss.str();
        if (b != ((m_table[i-1]).getChaine())) {
            string nom = ((m_table[i-1]).getChaine());
            cout << setw(4 * (indentation + 1)) << "" << "int " << nom << "; " << endl;
        }
        i = i + 1;
    }
    getArbre()->traduitEnCPP(cout, indentation + 1);
    cout << setw(4 * (indentation + 1)) << "" << "return 0;" << endl;
    cout << setw(4 * indentation) << "}" << endl;
}





