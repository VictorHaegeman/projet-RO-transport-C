#ifndef MARCHE_PIED_H
#define MARCHE_PIED_H

#include "problem.h"
#include "base.h"

// Applique un pas de marche-pied en utilisant l'arc entré (i_entree, j_entree)
// b : base actuelle (arbre) utilisée pour trouver le cycle
// s : solution (matrice des quantités x_ij), modifiée en place
// Renvoie theta (>=0) et, via pointeurs, l'arc sortant choisi
int marche_pied(const Base *b, Solution *s,
                int i_entree, int j_entree,
                int *i_sortie, int *j_sortie);


#endif
