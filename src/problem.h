#ifndef PROBLEM_H
#define PROBLEM_H

typedef struct {
    int nb_fournisseurs;
    int nb_clients;
    int **couts;         // matrice a_ij
    int *provisions;     // P_i
    int *commandes;      // C_j
} Probleme;

typedef struct {
    int nb_fournisseurs;
    int nb_clients;
    int **x;             // quantités transportées b_ij
} Solution;

Probleme *lire_probleme(const char *nom_fichier);
void afficher_probleme(const Probleme *p);

Solution *creer_solution_vide(const Probleme *p);
void afficher_solution(const Probleme *p, const Solution *s);

void liberer_probleme(Probleme *p);
void liberer_solution(Solution *s);

#endif
