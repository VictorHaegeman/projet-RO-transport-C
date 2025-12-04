#include <stdio.h>
#include <limits.h>
#include "marche_pied.h"

// On représente les nœuds du graphe biparti par des indices :
//  - Fournisseurs : 0 .. n-1
//  - Clients      : n .. n+m-1  (client j ↦ n + j)

// BFS pour trouver un chemin entre deux nœuds dans la base (qui est un arbre)
static int trouver_chemin(const Base *b, int n, int m,
                          int source, int cible, int parent[])
{
    int total = n + m;
    int file[64];   // suffisant pour nos petits exemples
    int tete = 0, queue = 0;

    for (int i = 0; i < total; i++)
        parent[i] = -1;

    file[queue++] = source;
    parent[source] = source;  // marqueur de racine

    while (tete < queue && parent[cible] == -1) {
        int u = file[tete++];

        for (int k = 0; k < b->nb_arcs; k++) {
            int fi = b->arcs[k][0];
            int cj = b->arcs[k][1];

            int a = fi;          // nœud fournisseur
            int c = n + cj;      // nœud client

            int v = -1;
            if (u == a)      v = c;
            else if (u == c) v = a;
            else            continue;

            if (parent[v] == -1) {
                parent[v] = u;
                file[queue++] = v;
            }
        }
    }

    return (parent[cible] != -1);
}

void marche_pied(const Base *b, Solution *s, int i_entree, int j_entree)
{
    int n = s->nb_fournisseurs;
    int m = s->nb_clients;
    int total = n + m;

    int noeud_f = i_entree;      // fournisseur
    int noeud_c = n + j_entree;  // client

    int parent[64];

    // 1) Trouver le chemin dans la base entre F_i_entree et C_j_entree
    if (!trouver_chemin(b, n, m, noeud_f, noeud_c, parent)) {
        printf("Erreur marche_pied : impossible de trouver un chemin entre F%d et C%d dans la base.\n",
               i_entree, j_entree);
        return;
    }

    // 2) Reconstruire le chemin (suite de nœuds) de F -> C
    int chemin_noeuds[64];
    int len = 0;
    int cur = noeud_c;

    while (1) {
        chemin_noeuds[len++] = cur;
        if (cur == noeud_f) break;
        cur = parent[cur];
    }

    // inverser pour avoir F ... C
    for (int i = 0; i < len / 2; i++) {
        int tmp = chemin_noeuds[i];
        chemin_noeuds[i] = chemin_noeuds[len - 1 - i];
        chemin_noeuds[len - 1 - i] = tmp;
    }

    // 3) Construire la liste des cases du cycle (i,j)
    //    Le cycle comprend :
    //      - l’arc entrant (i_entree, j_entree)
    //      - tous les arcs du chemin F -> C, dans l’ordre
    int cycle_taille = len;  // nb d'arcs
    int cycle_i[64];
    int cycle_j[64];

    // arc 0 : arc entrant
    cycle_i[0] = i_entree;
    cycle_j[0] = j_entree;

    // arcs 1.. : arcs de la base le long du chemin
    for (int e = 0; e < len - 1; e++) {
        int u = chemin_noeuds[e];
        int v = chemin_noeuds[e + 1];

        int fi, cj;

        if (u < n && v >= n) {         // u = fournisseur, v = client
            fi = u;
            cj = v - n;
        }
        else if (v < n && u >= n) {    // v = fournisseur, u = client
            fi = v;
            cj = u - n;
        }
        else {
            printf("Erreur marche_pied : chemin non biparti.\n");
            return;
        }

        cycle_i[e + 1] = fi;
        cycle_j[e + 1] = cj;
    }

    // 4) Déterminer les signes (+/-) le long du cycle :
    //    + sur l’arc entrant, puis alternance
    int signe[64];
    for (int e = 0; e < cycle_taille; e++) {
        signe[e] = (e % 2 == 0) ? +1 : -1;
    }

    // 5) Calcul de theta = min des x_ij sur les arcs marqués "−"
    int theta = INT_MAX;
    for (int e = 0; e < cycle_taille; e++) {
        if (signe[e] == -1) {
            int ii = cycle_i[e];
            int jj = cycle_j[e];
            int x = s->x[ii][jj];
            if (x < theta) theta = x;
        }
    }

    if (theta == INT_MAX) {
        printf("Erreur marche_pied : aucun arc avec signe '-' dans le cycle.\n");
        return;
    }

    printf("\n--- Marche-pied pour l’arc entrant (%d,%d) ---\n", i_entree, j_entree);
    printf("Cycle trouvé :\n");
    for (int e = 0; e < cycle_taille; e++) {
        printf("  %c (%d,%d)\n", (signe[e] > 0 ? '+' : '-'),
               cycle_i[e], cycle_j[e]);
    }
    printf("Theta = %d\n", theta);

    // 6) Mettre à jour la solution
    for (int e = 0; e < cycle_taille; e++) {
        int ii = cycle_i[e];
        int jj = cycle_j[e];
        s->x[ii][jj] += signe[e] * theta;
    }

    // 7) Identifier l’arc sortant (celui qui tombe à 0 parmi les “−”)
    int i_sortie = -1, j_sortie = -1;
    for (int e = 0; e < cycle_taille; e++) {
        if (signe[e] == -1) {
            int ii = cycle_i[e];
            int jj = cycle_j[e];
            if (s->x[ii][jj] == 0) {
                i_sortie = ii;
                j_sortie = jj;
                break;
            }
        }
    }

    if (i_sortie != -1) {
        printf("Arc sortant de la base : (%d,%d)\n", i_sortie, j_sortie);
    } else {
        printf("Attention : aucun arc '-' n’est tombé à 0 (situation dégénérée).\n");
    }
}
