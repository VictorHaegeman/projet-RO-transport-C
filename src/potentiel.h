#ifndef POTENTIEL_H
#define POTENTIEL_H

#include "problem.h"
#include "base.h"

/*
 * calculer_potentiels
 * - Entrées : problème p, base b
 * - Sorties : pot_f[i] = E(F_i), pot_c[j] = E(C_j)
 * - Idée : pour chaque arc basique (i,j), on impose
 *          cout_ij = E(F_i) - E(C_j) et on propage.
 */
void calculer_potentiels(const Probleme *p, const Base *b,
                         int *pot_f, int *pot_c);

/*
 * afficher_potentiels
 * - Affiche E(F_i) pour tous les fournisseurs
 *   et E(C_j) pour tous les clients.
 */
void afficher_potentiels(const Probleme *p,
                         const int *pot_f, const int *pot_c);

/*
 * afficher_table_couts_potentiels
 * - Affiche la table cp_ij = E(F_i) - E(C_j)
 *   (même format que la matrice de coûts).
 */
void afficher_table_couts_potentiels(const Probleme *p,
                                     const int *pot_f, const int *pot_c);

/*
 * calculer_et_afficher_couts_marginaux
 * - Calcule m_ij = cout_ij - (E(F_i) - E(C_j))
 * - Affiche la table des coûts marginaux
 * - Ne regarde que les cases hors base (x_ij == 0) :
 *      * si toutes ont m_ij >= 0 → solution optimale, renvoie 1
 *      * sinon → choisit la case avec m_ij le plus négatif,
 *                met ses indices dans (i_entree, j_entree),
 *                renvoie 0.
 */
int calculer_et_afficher_couts_marginaux(const Probleme *p,
                                         const Solution *s,
                                         const int *pot_f,
                                         const int *pot_c,
                                         int *i_entree,
                                         int *j_entree);

#endif
