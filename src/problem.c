#include <stdio.h>
#include <stdlib.h>
#include "problem.h"

Probleme *lire_probleme(const char *nom_fichier)
{
    FILE *f = fopen(nom_fichier, "r");
    if (!f) return NULL;

    Probleme *p = malloc(sizeof(Probleme));

    fscanf(f, "%d %d", &p->nb_fournisseurs, &p->nb_clients);

    // Allocation matrice de coûts
    p->couts = malloc(p->nb_fournisseurs * sizeof(int*));
    for (int i = 0; i < p->nb_fournisseurs; i++)
        p->couts[i] = malloc(p->nb_clients * sizeof(int));

    p->provisions = malloc(p->nb_fournisseurs * sizeof(int));
    p->commandes  = malloc(p->nb_clients * sizeof(int));

    // Lecture du tableau
    for (int i = 0; i < p->nb_fournisseurs; i++) {
        for (int j = 0; j < p->nb_clients; j++)
            fscanf(f, "%d", &p->couts[i][j]);
        fscanf(f, "%d", &p->provisions[i]);
    }

    for (int j = 0; j < p->nb_clients; j++)
        fscanf(f, "%d", &p->commandes[j]);

    fclose(f);
    return p;
}

void afficher_probleme(const Probleme *p)
{
    printf("=== PROBLÈME DE TRANSPORT ===\n");
    printf("Fournisseurs : %d | Clients : %d\n\n",
           p->nb_fournisseurs, p->nb_clients);

    for (int i = 0; i < p->nb_fournisseurs; i++) {
        for (int j = 0; j < p->nb_clients; j++)
            printf("%4d ", p->couts[i][j]);
        printf("| %d\n", p->provisions[i]);
    }

    for (int j = 0; j < p->nb_clients; j++)
        printf("-----");
    printf("\n");

    for (int j = 0; j < p->nb_clients; j++)
        printf("%4d ", p->commandes[j]);
    printf("\n\n");
}

Solution *creer_solution_vide(const Probleme *p)
{
    Solution *s = malloc(sizeof(Solution));
    s->nb_fournisseurs = p->nb_fournisseurs;
    s->nb_clients      = p->nb_clients;

    s->x = malloc(s->nb_fournisseurs * sizeof(int*));
    for (int i = 0; i < s->nb_fournisseurs; i++) {
        s->x[i] = malloc(s->nb_clients * sizeof(int));
        for (int j = 0; j < s->nb_clients; j++)
            s->x[i][j] = 0;
    }

    return s;
}

void afficher_solution(const Probleme *p, const Solution *s)
{
    printf("=== TABLEAU DE TRANSPORT ===\n");
    for (int i = 0; i < s->nb_fournisseurs; i++) {
        for (int j = 0; j < s->nb_clients; j++)
            printf("%4d ", s->x[i][j]);
        printf("\n");
    }
    printf("\n");
}

void liberer_probleme(Probleme *p)
{
    for (int i = 0; i < p->nb_fournisseurs; i++)
        free(p->couts[i]);
    free(p->couts);
    free(p->provisions);
    free(p->commandes);
    free(p);
}

void liberer_solution(Solution *s)
{
    for (int i = 0; i < s->nb_fournisseurs; i++)
        free(s->x[i]);
    free(s->x);
    free(s);
}