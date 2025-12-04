#ifndef BASE_H
#define BASE_H

#include "problem.h"   // contient normalement la définition de Solution

// Représente la base : liste des arcs (i,j) avec x[i][j] > 0
typedef struct {
    int nb_arcs;      // nombre d'arcs de la base
    int (*arcs)[2];   // arcs[k][0] = i (fournisseur), arcs[k][1] = j (client)
} Base;

// Construit la base à partir de la solution (on garde les x[i][j] > 0)
Base *construire_base(const Solution *s);

// Affichage simple : liste des arcs F_i -> C_j
void afficher_base_liste(const Base *b);

// Affichage "graphe" : fournisseurs en haut, clients en bas, arcs dessinés en ASCII
void afficher_base_graphe(const Base *b, int nb_fournisseurs, int nb_clients);

int base_est_arbre(const Base *b, int n, int m);
Base*corriger_base(const Base *b, const Solution *s, int nb_fournisseur, int nb_clients);

// Libère la mémoire de la base
void liberer_base(Base *b);

#endif
