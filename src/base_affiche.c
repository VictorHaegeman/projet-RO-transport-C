#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "base.h"
#include "trace.h"

#define LARGEUR_ASCII 200
#define HAUTEUR_ASCII 20
#define MARGE_HORIZ   4
#define ESPACE_COL    8   // espacement horizontal entre noeuds

// ---------- Fonction utilitaire pour écrire du texte dans une "grille" ASCII ----------
static void dessiner_texte(char canvas[HAUTEUR_ASCII][LARGEUR_ASCII],
                           int ligne, int colonne, const char *txt)
{
    for (int k = 0; txt[k] != '\0'; k++) {
        int c = colonne + k;
        if (ligne >= 0 && ligne < HAUTEUR_ASCII &&
            c >= 0 && c < LARGEUR_ASCII) {
            canvas[ligne][c] = txt[k];
        }
    }
}

// ---------- Construction de la base ----------

Base *construire_base(const Solution *s)
{
    Base *b = malloc(sizeof(Base));
    if (!b) return NULL;

    int compteur = 0;
    for (int i = 0; i < s->nb_fournisseurs; i++) {
        for (int j = 0; j < s->nb_clients; j++) {
            if (s->x[i][j] > 0)
                compteur++;
        }
    }

    b->nb_arcs = compteur;
    b->arcs = NULL;

    if (compteur == 0) {
        // Base vide (cas théorique)
        return b;
    }

    b->arcs = malloc(sizeof(int[2]) * compteur);
    if (!b->arcs) {
        free(b);
        return NULL;
    }

    int k = 0;
    for (int i = 0; i < s->nb_fournisseurs; i++) {
        for (int j = 0; j < s->nb_clients; j++) {
            if (s->x[i][j] > 0) {
                b->arcs[k][0] = i;   // fournisseur
                b->arcs[k][1] = j;   // client
                k++;
            }
        }
    }

    return b;
}

// ---------- Affichage simple (liste d'arcs) ----------

void afficher_base_liste(const Base *b)
{
    trace("\n=== BASE (LISTE DES ARCS) ===\n");
    if (!b || b->nb_arcs == 0) {
        trace("(base vide)\n");
        trace("=============================\n");
        return;
    }

    for (int k = 0; k < b->nb_arcs; k++) {
        int i = b->arcs[k][0];
        int j = b->arcs[k][1];
        trace("Arc %d : F%d -> C%d\n", k, i, j);
    }
    trace("Nombre d'arcs = %d\n", b->nb_arcs);
    trace("=============================\n");
}

// ---------- Affichage "graphe" comme dans le cours ----------
//
// Rangée du haut : F0   F1   F2  ...
// Rangée du bas : T0   T1   T2  ...
// Entre les deux : des diagonales / \ qui relient F vers T.
//

void afficher_base_graphe(const Base *b, int nb_fournisseurs, int nb_clients)
{
    printf("\n=== GRAPHE DESSINE ===\n\n");

    if (!b || b->nb_arcs == 0) {
        printf("(base vide)\n");
        printf("=============================================\n");
        return;
    }

    // 1) Initialisation de la "feuille" ASCII
    char canvas[HAUTEUR_ASCII][LARGEUR_ASCII];
    for (int r = 0; r < HAUTEUR_ASCII; r++) {
        for (int c = 0; c < LARGEUR_ASCII; c++) {
            canvas[r][c] = ' ';
        }
    }

    int ligne_haut = 2;                 
    int ligne_bas  = HAUTEUR_ASCII - 3; 

    // 2) Positions horizontales des fournisseurs et des clients
    int posF[nb_fournisseurs];
    int posC[nb_clients];

    for (int i = 0; i < nb_fournisseurs; i++) {
        posF[i] = MARGE_HORIZ + i * ESPACE_COL;
        char buf[8];
        snprintf(buf, sizeof(buf), "S%d", i+1);
        dessiner_texte(canvas, ligne_haut, posF[i], buf);
    }

    for (int j = 0; j < nb_clients; j++) {
        posC[j] = MARGE_HORIZ + j * ESPACE_COL;
        char buf[8];
        snprintf(buf, sizeof(buf), "T%c", 'a' + j);
        dessiner_texte(canvas, ligne_bas, posC[j], buf);
    }

    // 3) Tracer les arcs
    for (int k = 0; k < b->nb_arcs; k++) {
        int i = b->arcs[k][0];
        int j = b->arcs[k][1];

        int x1 = posF[i] + 1;
        int y1 = ligne_haut + 1;
        int x2 = posC[j] + 1;
        int y2 = ligne_bas - 1;

        int dx = x2 - x1;
        int dy = y2 - y1;
        int steps = (dy > 0) ? dy : -dy;
        if (steps == 0) steps = 1;

        for (int s = 0; s <= steps; s++) {
            int y = y1 + (dy * s) / steps;
            int x = x1 + (dx * s) / steps;

            if (y < 0 || y >= HAUTEUR_ASCII || x < 0 || x >= LARGEUR_ASCII)
                continue;

            char ch;
            if (dx == 0)
                ch = '|';
            else if (dx > 0)
                ch = '\\';
            else
                ch = '/';

            if (canvas[y][x] == ' ')
                canvas[y][x] = ch;
            else if (canvas[y][x] == '/' || canvas[y][x] == '\\' || canvas[y][x] == '|')
                canvas[y][x] = '*';
        }
    }

    // 4) Affichage final
    for (int r = 0; r < HAUTEUR_ASCII; r++) {
        fwrite(canvas[r], 1, LARGEUR_ASCII, stdout);
        putchar('\n');
    }

    printf("\n=============================================\n");
}

// ================================================================
// ===============   TEST SI LA BASE EST UN ARBRE   ===============
// ================================================================

// Graphe biparti : fournisseurs = 0..(n-1), clients = n..(n+m-1)
//
// On crée un tableau d’adjacence F-C, C-F et on fait une DFS.

static void dfs(int noeud, int visited[], int n, int m, const Base *b)
{
    visited[noeud] = 1;

    // Explorer tous les arcs de la base
    for (int k = 0; k < b->nb_arcs; k++) {
        int fi = b->arcs[k][0];  // fournisseur
        int cj = b->arcs[k][1];  // client

        int u = fi;
        int v = n + cj; // client mappé à un numéro différent

        if (noeud == u && !visited[v])
            dfs(v, visited, n, m, b);
        if (noeud == v && !visited[u])
            dfs(u, visited, n, m, b);
    }
}

int base_est_arbre(const Base *b, int n, int m)
{
    // ---------- Test 1 : nombre d'arcs ----------
    if (b->nb_arcs != n + m - 1) {
        return 0; // faux : pas le bon nombre d'arcs
    }

    // ---------- Test 2 : connexité ----------
    int total = n + m;
    int visited[total];
    for (int i = 0; i < total; i++)
        visited[i] = 0;

    // Lancer un DFS depuis le noeud 0 (F0)
    dfs(0, visited, n, m, b);

    // Vérifier que tous les noeuds ont été visités
    for (int i = 0; i < total; i++) {
        if (!visited[i])
            return 0; // base non connexe → pas un arbre
    }

    // Si on arrive ici : nombre d'arcs correct + connexité = ARBRE
    return 1;
}

// ================================================================
// ==========   CORRECTION : SUPPRIMER UN CYCLE  ==================
// ================================================================
//
// Idée : si la base contient un cycle (nb_arcs >= n+m), on cherche
// un cycle simple dans le graphe biparti, on alterne + / - le long
// du cycle, on pousse un flux theta = min sur les arcs "−".
// Au moins un arc tombe à 0 → on récupère une base arborescente.

// Retourne 1 si un cycle a été trouvé et remplit cycle_noeuds (avec nœud initial répété à la fin)
static int dfs_cycle(const Base *b, int n, int m, int u, int parent_node,
                     int visited[], int parent[],
                     int cycle_noeuds[], int *cycle_len)
{
    int total_nodes = n + m;
    visited[u] = 1;

    for (int k = 0; k < b->nb_arcs; k++) {
        int fi = b->arcs[k][0];
        int cj = b->arcs[k][1];
        int v = -1;
        if (u == fi) v = n + cj;
        else if (u == n + cj) v = fi;
        else continue;

        if (!visited[v]) {
            parent[v] = u;
            if (dfs_cycle(b, n, m, v, u, visited, parent, cycle_noeuds, cycle_len))
                return 1;
        } else if (v != parent_node) {
            // Cycle trouvé (arête de retour u - v)
            int temp[total_nodes];
            int len = 0;
            int cur = u;
            temp[len++] = u;
            while (cur != v && cur != -1) {
                cur = parent[cur];
                temp[len++] = cur;
            }
            if (cur == -1) continue; // sécurité

            int idx = 0;
            cycle_noeuds[idx++] = v;
            for (int t = 0; t < len; t++)
                cycle_noeuds[idx++] = temp[t];
            cycle_noeuds[idx++] = v; // refermer

            *cycle_len = idx;
            return 1;
        }
    }
    return 0;
}

static int trouver_cycle(const Base *b, int n, int m, int cycle_noeuds[], int *cycle_len)
{
    int total = n + m;
    int visited[total];
    int parent[total];

    for (int i = 0; i < total; i++) {
        visited[i] = 0;
        parent[i] = -1;
    }

    for (int start = 0; start < total; start++) {
        if (!visited[start]) {
            if (dfs_cycle(b, n, m, start, -1, visited, parent, cycle_noeuds, cycle_len))
                return 1;
        }
    }
    return 0;
}

static int arc_existe(const Base *b, int nb_arcs_utilises, int fi, int cj)
{
    for (int k = 0; k < nb_arcs_utilises; k++) {
        if (b->arcs[k][0] == fi && b->arcs[k][1] == cj)
            return 1;
    }
    return 0;
}

static int find_parent(int x, int parent_conn[])
{
    if (parent_conn[x] != x)
        parent_conn[x] = find_parent(parent_conn[x], parent_conn);
    return parent_conn[x];
}

static void unite_parent(int a, int b, int parent_conn[])
{
    int ra = find_parent(a, parent_conn);
    int rb = find_parent(b, parent_conn);
    if (ra != rb) parent_conn[rb] = ra;
}

Base *corriger_base(const Base *b, Solution *s, int n, int m)
{
    int total = n + m;
    int max_cycle = total + 2; // borne haute pour stocker le cycle (noeud répété)

    if (!b) return construire_base(s);

    // Pas de correction si déjà un arbre
    if (base_est_arbre(b, n, m)) return construire_base(s);

    // Cas 1 : base non connexe / trop peu d'arcs. On ajoute des arcs de valeur 0 pour obtenir n+m-1 arcs.
    if (b->nb_arcs < n + m - 1) {
        trace("Base non connexe (nb_arcs=%d < %d). Ajout d'arcs nuls pour connecter.\n", b->nb_arcs, n + m - 1);

        // Tableau pour savoir si un noeud est incident à au moins un arc
        int deg[total];
        for (int t = 0; t < total; t++) deg[t] = 0;
        for (int k = 0; k < b->nb_arcs; k++) {
            int fi = b->arcs[k][0];
            int cj = b->arcs[k][1];
            deg[fi]++;
            deg[n + cj]++;
        }

        Base *nb = malloc(sizeof(Base));
        nb->arcs = malloc(sizeof(int[2]) * (n + m - 1));
        nb->nb_arcs = 0;

        // Copier les arcs existants
        for (int k = 0; k < b->nb_arcs; k++) {
            nb->arcs[nb->nb_arcs][0] = b->arcs[k][0];
            nb->arcs[nb->nb_arcs][1] = b->arcs[k][1];
            nb->nb_arcs++;
        }

        // 0) Assurer la connexité en priorité tant qu'il reste des emplacements
        int parent_conn[total];
        for (int i = 0; i < total; i++) parent_conn[i] = i;
        for (int k = 0; k < nb->nb_arcs; k++) {
            int fi = nb->arcs[k][0];
            int cj = nb->arcs[k][1];
            unite_parent(fi, n + cj, parent_conn);
        }
        int root0 = find_parent(0, parent_conn);
        for (int node = 0; node < total && nb->nb_arcs < n + m - 1; node++) {
            if (find_parent(node, parent_conn) != root0) {
                int fi, cj;
                if (node < n) { fi = node; cj = 0; }
                else         { fi = 0;    cj = node - n; }

                if (!arc_existe(nb, nb->nb_arcs, fi, cj)) {
                    nb->arcs[nb->nb_arcs][0] = fi;
                    nb->arcs[nb->nb_arcs][1] = cj;
                    nb->nb_arcs++;
                    unite_parent(fi, n + cj, parent_conn);
                    root0 = find_parent(0, parent_conn);
                }
            }
        }

        // 1) Connecter chaque client isolé à un fournisseur (ici F0)
        for (int cj = 0; cj < m && nb->nb_arcs < n + m - 1; cj++) {
            if (deg[n + cj] == 0) {
                int fi = 0;
                if (!arc_existe(nb, nb->nb_arcs, fi, cj)) {
                    nb->arcs[nb->nb_arcs][0] = fi;
                    nb->arcs[nb->nb_arcs][1] = cj;
                    nb->nb_arcs++;
                }
            }
        }

        // 2) Connecter chaque fournisseur isolé à un client (ici C0)
        for (int fi = 0; fi < n && nb->nb_arcs < n + m - 1; fi++) {
            if (deg[fi] == 0) {
                int cj = 0;
                if (!arc_existe(nb, nb->nb_arcs, fi, cj)) {
                    nb->arcs[nb->nb_arcs][0] = fi;
                    nb->arcs[nb->nb_arcs][1] = cj;
                    nb->nb_arcs++;
                }
            }
        }

        // 3) Si toujours pas assez d'arcs, ajouter des arcs distincts simples pour atteindre n+m-1
        for (int fi = 0; fi < n && nb->nb_arcs < n + m - 1; fi++) {
            for (int cj = 0; cj < m && nb->nb_arcs < n + m - 1; cj++) {
                if (!arc_existe(nb, nb->nb_arcs, fi, cj)) {
                    nb->arcs[nb->nb_arcs][0] = fi;
                    nb->arcs[nb->nb_arcs][1] = cj;
                    nb->nb_arcs++;
                }
            }
        }

        trace("Base corrigée avec %d arcs (ajouts nuls) pour obtenir un arbre.\n", nb->nb_arcs);
        // Les x correspondants restent à 0 dans s->x (degenerescence)
        return nb;
    }

    int cycle_noeuds[max_cycle];
    int cycle_len = 0;

    if (!trouver_cycle(b, n, m, cycle_noeuds, &cycle_len) || cycle_len < 4) {
        trace("Aucun cycle détecté (ou cycle trop court) alors que nb_arcs=%d.\n", b->nb_arcs);
        return construire_base(s);
    }

    int nb_arcs_cycle = cycle_len - 1; // dernier noeud répété
    int cycle_i[max_cycle];
    int cycle_j[max_cycle];
    int signe[max_cycle];

    // Construire la liste des arcs du cycle
    for (int e = 0; e < nb_arcs_cycle; e++) {
        int u = cycle_noeuds[e];
        int v = cycle_noeuds[e + 1];

        int fi, cj;
        if (u < n && v >= n) { fi = u; cj = v - n; }
        else if (v < n && u >= n) { fi = v; cj = u - n; }
        else {
            trace("Cycle non biparti détecté, abandon de la correction.\n");
            return construire_base(s);
        }
        cycle_i[e] = fi;
        cycle_j[e] = cj;
        signe[e] = (e % 2 == 0) ? +1 : -1; // + - + - ...
    }

    // Trouver theta = min des x sur arcs marqués "-"
    int theta = INT_MAX;
    for (int e = 0; e < nb_arcs_cycle; e++) {
        if (signe[e] == -1) {
            int x = s->x[cycle_i[e]][cycle_j[e]];
            if (x < theta) theta = x;
        }
    }

    if (theta == INT_MAX) {
        trace("Correction : aucun arc '-' ? Impossible de pousser un flux.\n");
        return construire_base(s);
    }

    trace("\n>>> Correction de cycle détecté (nb_arcs=%d). Cycle :\n", b->nb_arcs);
    for (int e = 0; e < nb_arcs_cycle; e++) {
        trace("  %c (%d,%d)\n", signe[e] > 0 ? '+' : '-', cycle_i[e], cycle_j[e]);
    }
    trace("Theta choisi = %d\n", theta);

    // Appliquer +theta / -theta le long du cycle
    for (int e = 0; e < nb_arcs_cycle; e++) {
        s->x[cycle_i[e]][cycle_j[e]] += signe[e] * theta;
    }

    // Identifier l'arc qui sort (tombé à 0)
    int i_sortie = -1, j_sortie = -1;
    for (int e = 0; e < nb_arcs_cycle; e++) {
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
        trace("Arc retiré de la base : (%d,%d)\n", i_sortie, j_sortie);
    } else {
        trace("Attention : aucun arc '-' n'est tombé à 0 (dégénérescence possible).\n");
    }

    // Re-construire la base à partir de la solution modifiée
    return construire_base(s);
}

// ---------- Libération ----------

void liberer_base(Base *b)
{
    if (!b) return;
    if (b->arcs) free(b->arcs);
    free(b);
}