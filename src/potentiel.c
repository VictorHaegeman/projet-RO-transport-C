#include <stdio.h>
#include <limits.h>
#include "potentiel.h"
#include "trace.h"

/*
 * on prend un sommet de depart :
 * on choisi un qui est tres connecter.
 *
 * Les fournisseurs sont numérotés 0 à n-1
 * Les clients      sont numérotés n à n+m-1
 *
 * La fonction renvoie l'indice du sommet choisi.
 */
static int choisir_sommet_depart(const Base *b, int n, int m)
{
    int total = n + m;
    int degres[64];   /* on part du principe que ca sera suffisant  */
    int i;

    /* Initialisation des degrés à 0 */
    // Degres c est le nombre de connexion quil a 
    for (i = 0; i < total; i++)
        degres[i] = 0;

    /* Comptage des degrés à partir de la base */
    if (b) {
        for (i = 0; i < b->nb_arcs; i++) {
            int fi = b->arcs[i][0];      /* fournisseur */
            int cj = b->arcs[i][1];      /* client */
            int u = fi;
            int v = n + cj;

            if (u >= 0 && u < total) degres[u]++;
            if (v >= 0 && v < total) degres[v]++;
        }
    }

    /* Choix du sommet de degré max */
    int sommet = 0;
    int deg_max = -1;
    for (i = 0; i < total; i++) {
        if (degres[i] > deg_max) {
            deg_max = degres[i];
            sommet = i;
        }
    }

    return sommet;
}

/*
 * Calcule les potentiels E(F_i) -> fournisseur et E(C_j) -> commandes à partir de la base.
 * On suppose que la base correspond à une proposition non dégénérée 
 * (graphe connexe et acyclique).
 */
void calculer_potentiels(const Probleme *p, const Base *b,
                         int *pot_f, int *pot_c)
{
    if (!p || !b || !pot_f || !pot_c)
        return;

    int n = p->nb_fournisseurs;
    int m = p->nb_clients;
    int total = n + m;

    /* On travaille d'abord sur un tableau de potentiels "par sommet" */
    int pot_sommet[64];
    int visite[64];
    int file[64];
    int tete = 0, queue = 0;

    if (total > 64) {
        
        // secu si ca depasse 64
        trace("Erreur : nombre total de sommets trop grand pour calculer les potentiels.\n");
        return;
    }

    int i;
    for (i = 0; i < total; i++) {
        pot_sommet[i] = 0;
        visite[i] = 0;
    }

    /* Choix du sommet de départ : celui qui a le plus de liaisons */
    int racine = choisir_sommet_depart(b, n, m);
    visite[racine] = 1;
    pot_sommet[racine] = 0;  /* on va fixer sa valeur à 0 */

    file[queue++] = racine;

    /* Parcours en largeur sur le graphe biparti défini par la base */
    while (tete < queue) {
        int u = file[tete++];
        int est_fournisseur = (u < n);

        for (i = 0; i < b->nb_arcs; i++) {
            int fi = b->arcs[i][0];
            int cj = b->arcs[i][1];
            int noeud_f = fi;
            int noeud_c = n + cj;

            int v;
            int cout = p->couts[fi][cj];

            if (est_fournisseur && u == noeud_f) {
                /* u  = fournisseur, v = client */
                v = noeud_c;
                if (!visite[v]) {
                    /* cout = E(F_i) - E(C_j) */
                    pot_sommet[v] = pot_sommet[u] - cout;
                    visite[v] = 1;
                    file[queue++] = v;
                }
            } else if (!est_fournisseur && u == noeud_c) {
                v = noeud_f;
                if (!visite[v]) {
                    /* cout = E(F_i) - E(C_j)  =>  E(F_i) = cout + E(C_j) : c'est la qu'est defini mon nouveau coup */
                    pot_sommet[v] = cout + pot_sommet[u];
                    visite[v] = 1;
                    file[queue++] = v;
                }
            }
        }
    }

    /* Recopie dans les tableaux de sortie */
    for (i = 0; i < n; i++)
        pot_f[i] = pot_sommet[i];

    for (i = 0; i < m; i++)
        pot_c[i] = pot_sommet[n + i];
}


 //Affiche les potentiels pour chaque sommet.

void afficher_potentiels(const Probleme *p,
                         const int *pot_f, const int *pot_c)
{
    if (!p || !pot_f || !pot_c)
        return;

    int n = p->nb_fournisseurs;
    int m = p->nb_clients;
    int i;

    trace("\n=== POTENTIELS ===\n");

    trace("Fournisseurs :\n");
    for (i = 0; i < n; i++) {
        trace("  E(F%d) = %d\n", i, pot_f[i]);
    }

    trace("Clients :\n");
    for (i = 0; i < m; i++) {
        trace("  E(C%d) = %d\n", i, pot_c[i]);
    }

    trace("==================\n\n");
}

/*
 * Affiche la table des coûts potentiels cp_ij = E(F_i) - E(C_j).
 */
void afficher_table_couts_potentiels(const Probleme *p,
                                     const int *pot_f, const int *pot_c)
{
    if (!p || !pot_f || !pot_c)
        return;

    int n = p->nb_fournisseurs;
    int m = p->nb_clients;

    int i, j;

    trace("=== TABLE DES COUTS POTENTIELS ===\n\n");

    /* (clients) */
    trace("      ");
    for (j = 0; j < m; j++) {
        trace("   C%-4d", j);
    }
    trace("\n");

    /* (fournisseurs) */
    for (i = 0; i < n; i++) {
        trace("F%-3d ", i);
        for (j = 0; j < m; j++) {
            int cp = pot_f[i] - pot_c[j];
            trace("%7d", cp);
        }
        trace("\n");
    }

    trace("\n");
}

/*
 * Calcule et affiche la table des coûts marginaux)
 * et dis si éventuellement une arête améliorante possible.
 */
static int arc_dans_base(const Base *b, int i, int j)
{
    if (!b)
        return 0;

    for (int k = 0; k < b->nb_arcs; k++) {
        if (b->arcs[k][0] == i && b->arcs[k][1] == j)
            return 1;
    }
    return 0;
}

int calculer_et_afficher_couts_marginaux(const Probleme *p,
                                         const Solution *s,
                                         const Base *b,
                                         const int *pot_f,
                                         const int *pot_c,
                                         int *i_entree,
                                         int *j_entree)
{
    if (!p || !s || !pot_f || !pot_c)
        return 1; /* on ne peut rien dire, supposons optimal */

    int n = p->nb_fournisseurs;
    int m = p->nb_clients;

    int i, j;

    int meilleur_i = -1;
    int meilleur_j = -1;
    int meilleur_marginal = 0;   /* on cherche un coût marginal STRICTEMENT négatif */

    trace("=== TABLE DES COUTS MARGINAUX ===\n\n");

    /* (clients) */
    trace("      ");
    for (j = 0; j < m; j++) {
        trace("   C%-4d", j);
    }
    trace("\n");

    for (i = 0; i < n; i++) {
        trace("F%-3d ", i);
        for (j = 0; j < m; j++) {
            int cp = pot_f[i] - pot_c[j];
            int marginal = p->couts[i][j] - cp;

            trace("%7d", marginal);

            /* On ne considère que les cases hors base (pas présentes dans b) pour l'arête améliorante */
            if (!arc_dans_base(b, i, j) && marginal < meilleur_marginal) {
                meilleur_marginal = marginal;
                meilleur_i = i;
                meilleur_j = j;
            }
        }
        trace("\n");
    }

    trace("\n");

    if (meilleur_i == -1) {
        trace("Aucune arête améliorante détectée : tous les coûts marginaux des cases hors base sont >= 0.\n");
        trace("La proposition de transport est optimale pour ce problème.\n\n");
        return 1;  /* optimale */
    }

    trace("Arête améliorante retenue : (F%d, C%d) avec coût marginal %d.\n\n",
           meilleur_i, meilleur_j, meilleur_marginal);

    if (i_entree)
        *i_entree = meilleur_i;
    if (j_entree)
        *j_entree = meilleur_j;

    return 0;  /* non optimale */
}
