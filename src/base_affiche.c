#include <stdio.h>
#include <stdlib.h>
#include "base.h"

#define LARGEUR_ASCII 120
#define HAUTEUR_ASCII 20
#define MARGE_HORIZ   4
#define ESPACE_COL    10   // espacement horizontal entre noeuds

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
    printf("\n=== BASE (LISTE DES ARCS) ===\n");
    if (!b || b->nb_arcs == 0) {
        printf("(base vide)\n");
        printf("=============================\n");
        return;
    }

    for (int k = 0; k < b->nb_arcs; k++) {
        int i = b->arcs[k][0];
        int j = b->arcs[k][1];
        printf("Arc %d : F%d -> C%d\n", k, i, j);
    }
    printf("Nombre d'arcs = %d\n", b->nb_arcs);
    printf("=============================\n");
}

// ---------- Affichage "graphe" façon schéma du prof ----------
//
// Rangée du haut : F0   F1   F2  ...
// Rangée du bas : T0   T1   T2  ...
// Entre les deux : des diagonales / \ qui relient F vers T.
//

void afficher_base_graphe(const Base *b, int nb_fournisseurs, int nb_clients)
{
    printf("\n=== GRAPHE BIPARTI (STYLE SCHÉMA DU PROF) ===\n\n");

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

// ---------- Libération ----------

void liberer_base(Base *b)
{
    if (!b) return;
    if (b->arcs) free(b->arcs);
    free(b);
}
