#include <stdio.h>
#include <limits.h>
#include "balas_hammer.h"
#include "trace.h"

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

    trace("\n--- Déroulé de la méthode de Balas-Hammer ---\n\n");

    while (1)
    {
        /* Nombre de lignes/colonnes encore vraiment utilisables (quantités > 0) */
        int nb_lignes_actives = 0;
        int nb_colonnes_actives = 0;

        for (int i = 0; i < p->nb_fournisseurs; i++) {
            if (ligne_active[i] && provisions[i] > 0)
                nb_lignes_actives++;
            else if (provisions[i] == 0)
                ligne_active[i] = 0; /* nettoyage au passage */
        }
        for (int j = 0; j < p->nb_clients; j++) {
            if (colonne_active[j] && commandes[j] > 0)
                nb_colonnes_actives++;
            else if (commandes[j] == 0)
                colonne_active[j] = 0;
        }

        if (nb_lignes_actives + nb_colonnes_actives <= 1)
            break;

        int meilleure_penalite = -1;
        int type = 0;   // 0 = ligne, 1 = colonne
        int indice = -1;

        // Pénalités lignes
        for (int i = 0; i < p->nb_fournisseurs; i++) {
            if (!ligne_active[i] || provisions[i] == 0) continue;
            int pen_ligne = penalite_ligne(p, i);
            if (pen_ligne > meilleure_penalite) {
                meilleure_penalite = pen_ligne;
                type = 0;
                indice = i;
            }
        }

        // Pénalités colonnes
        for (int j = 0; j < p->nb_clients; j++) {
            if (!colonne_active[j] || commandes[j] == 0) continue;
            int pen_colonne = penalite_colonne(p, j);
            if (pen_colonne > meilleure_penalite) {
                meilleure_penalite = pen_colonne;
                type = 1;
                indice = j;
            }
        }

        trace("Pénalité maximale : %d → %s %d\n",
               meilleure_penalite,
               type == 0 ? "ligne" : "colonne",
               indice);

        int meilleur_i = -1;
        int meilleur_j = -1;
        int meilleur_cout = INT_MAX;

        if (type == 0) {
            for (int j = 0; j < p->nb_clients; j++) {
                if (!colonne_active[j] || commandes[j] == 0) continue;
                if (p->couts[indice][j] < meilleur_cout) {
                    meilleur_cout = p->couts[indice][j];
                    meilleur_i = indice;
                    meilleur_j = j;
                }
            }
        } else {
            for (int i = 0; i < p->nb_fournisseurs; i++) {
                if (!ligne_active[i] || provisions[i] == 0) continue;
                if (p->couts[i][indice] < meilleur_cout) {
                    meilleur_cout = p->couts[i][indice];
                    meilleur_i = i;
                    meilleur_j = indice;
                }
            }
        }
        if (meilleur_i == -1 || meilleur_j == -1) {
            trace("Erreur BH : aucune cellule valide trouvée (base complète ? données incohérentes ?)\n");
            break;   // ou return ;
        }
        

        int q = minimum(provisions[meilleur_i], commandes[meilleur_j]);
        s->x[meilleur_i][meilleur_j] = q;

        trace("Remplissage de la case (%d,%d) avec %d unités (coût %d)\n",
               meilleur_i, meilleur_j, q, meilleur_cout);

        provisions[meilleur_i] -= q;
        commandes[meilleur_j]  -= q;

        /* Éviter de rayer simultanément la ligne ET la colonne en cas d’égalité */
        if (provisions[meilleur_i] == 0 && commandes[meilleur_j] == 0) {
            int pen_ligne = penalite_ligne(p, meilleur_i);
            int pen_col  = penalite_colonne(p, meilleur_j);

            if (pen_ligne > pen_col) {
                colonne_active[meilleur_j] = 0;
            } else if (pen_col > pen_ligne) {
                ligne_active[meilleur_i] = 0;
            } else {
                /* À égalité parfaite, on garde la dimension qui offre le plus d’options restantes */
                if (nb_lignes_actives >= nb_colonnes_actives)
                    colonne_active[meilleur_j] = 0;
                else
                    ligne_active[meilleur_i] = 0;
            }
        } else {
            if (provisions[meilleur_i] == 0)
                ligne_active[meilleur_i] = 0;
            if (commandes[meilleur_j] == 0)
                colonne_active[meilleur_j] = 0;
        }
    }

    trace("\n--- Fin de la méthode de Balas-Hammer ---\n\n");
}
