#include <stdio.h>
#include <limits.h>
#include "balas_hammer.h"

static int minimum(int a, int b) { return (a < b) ? a : b; }

// Trouve la pénalité d’une ligne
static int penalite_ligne(const Probleme *p, int ligne) {
    int min1 = INT_MAX, min2 = INT_MAX;

    for (int j = 0; j < p->nb_clients; j++) {
        int c = p->couts[ligne][j];
        if (c < min1) { min2 = min1; min1 = c; }
        else if (c < min2) min2 = c;
    }
    return min2 - min1;
}

// Trouve la pénalité d’une colonne
static int penalite_colonne(const Probleme *p, int col) {
    int min1 = INT_MAX, min2 = INT_MAX;

    for (int i = 0; i < p->nb_fournisseurs; i++) {
        int c = p->couts[i][col];
        if (c < min1) { min2 = min1; min1 = c; }
        else if (c < min2) min2 = c;
    }
    return min2 - min1;
}

void balas_hammer(const Probleme *p, Solution *s)
{
    int provisions[p->nb_fournisseurs];
    int commandes[p->nb_clients];

    for (int i = 0; i < p->nb_fournisseurs; i++)
        provisions[i] = p->provisions[i];

    for (int j = 0; j < p->nb_clients; j++)
        commandes[j] = p->commandes[j];

    int ligne_active[p->nb_fournisseurs];
    int colonne_active[p->nb_clients];

    for (int i = 0; i < p->nb_fournisseurs; i++)
        ligne_active[i] = 1;

    for (int j = 0; j < p->nb_clients; j++)
        colonne_active[j] = 1;

    printf("\n--- Déroulé de la méthode de Balas-Hammer ---\n\n");

    int nb_restants = p->nb_fournisseurs + p->nb_clients;

    while (nb_restants > 1)
    {
        int meilleure_penalite = -1;
        int type = 0;   // 0 = ligne, 1 = colonne
        int indice = -1;

        // Pénalités lignes
        for (int i = 0; i < p->nb_fournisseurs; i++) {
            if (!ligne_active[i]) continue;
            int pen = penalite_ligne(p, i);
            if (pen > meilleure_penalite) {
                meilleure_penalite = pen;
                type = 0;
                indice = i;
            }
        }

        // Pénalités colonnes
        for (int j = 0; j < p->nb_clients; j++) {
            if (!colonne_active[j]) continue;
            int pen = penalite_colonne(p, j);
            if (pen > meilleure_penalite) {
                meilleure_penalite = pen;
                type = 1;
                indice = j;
            }
        }

        printf("Pénalité maximale : %d → %s %d\n",
               meilleure_penalite,
               type == 0 ? "ligne" : "colonne",
               indice);

        int meilleur_i = -1;
        int meilleur_j = -1;
        int meilleur_cout = INT_MAX;

        if (type == 0) {
            for (int j = 0; j < p->nb_clients; j++) {
                if (!colonne_active[j]) continue;
                if (p->couts[indice][j] < meilleur_cout) {
                    meilleur_cout = p->couts[indice][j];
                    meilleur_i = indice;
                    meilleur_j = j;
                }
            }
        } else {
            for (int i = 0; i < p->nb_fournisseurs; i++) {
                if (!ligne_active[i]) continue;
                if (p->couts[i][indice] < meilleur_cout) {
                    meilleur_cout = p->couts[i][indice];
                    meilleur_i = i;
                    meilleur_j = indice;
                }
            }
        }

        int q = minimum(provisions[meilleur_i], commandes[meilleur_j]);
        s->x[meilleur_i][meilleur_j] = q;

        printf("Remplissage de la case (%d,%d) avec %d unités (coût %d)\n",
               meilleur_i, meilleur_j, q, meilleur_cout);

        provisions[meilleur_i] -= q;
        commandes[meilleur_j]  -= q;

        if (provisions[meilleur_i] == 0) {
            ligne_active[meilleur_i] = 0;
            nb_restants--;
        }
        if (commandes[meilleur_j] == 0) {
            colonne_active[meilleur_j] = 0;
            nb_restants--;
        }
    }

    printf("\n--- Fin de la méthode de Balas-Hammer ---\n\n");
}