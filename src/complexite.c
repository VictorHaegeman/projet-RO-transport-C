#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include "problem.h"
#include "nord_ouest.h"
#include "balas_hammer.h"
#include "base.h"

/*
 * Outils internes -----------------------------------------------------------
 */

/* Redirige temporairement stdout vers /dev/null pour rendre les algorithmes silencieux. */
static int silence_stdout(int *saved_fd)
{
    if (!saved_fd)
        return -1;

    fflush(stdout);
    int duplicate = dup(fileno(stdout));
    if (duplicate < 0)
        return -1;

    int null_fd = open("/dev/null", O_WRONLY);
    if (null_fd < 0) {
        close(duplicate);
        return -1;
    }

    if (dup2(null_fd, fileno(stdout)) < 0) {
        close(duplicate);
        close(null_fd);
        return -1;
    }

    close(null_fd);
    *saved_fd = duplicate;
    return 0;
}

static void restaurer_stdout(int saved_fd)
{
    if (saved_fd < 0)
        return;

    fflush(stdout);
    dup2(saved_fd, fileno(stdout));
    close(saved_fd);
}

static int **allouer_matrice_int(int n, int m)
{
    int **mat = calloc((size_t)n, sizeof(int *));
    if (!mat)
        return NULL;

    for (int i = 0; i < n; i++) {
        mat[i] = malloc((size_t)m * sizeof(int));
        if (!mat[i]) {
            for (int k = 0; k < i; k++)
                free(mat[k]);
            free(mat);
            return NULL;
        }
    }

    return mat;
}

/*
 * Génération / destruction d'un problème aléatoire -------------------------
 */

Probleme *generer_probleme_aleatoire(int n)
{
    if (n <= 0)
        return NULL;

    Probleme *p = calloc(1, sizeof(Probleme));
    if (!p)
        return NULL;

    p->nb_fournisseurs = n;
    p->nb_clients = n;

    p->couts = allouer_matrice_int(n, n);
    p->provisions = calloc((size_t)n, sizeof(int));
    p->commandes = calloc((size_t)n, sizeof(int));

    if (!p->couts || !p->provisions || !p->commandes) {
        if (p->couts) {
            for (int i = 0; i < n; i++)
                free(p->couts[i]);
            free(p->couts);
        }
        free(p->provisions);
        free(p->commandes);
        free(p);
        return NULL;
    }

    /* Génération de la matrice temporaire implicite pour provisions/commandes. */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int val = (rand() % 100) + 1;  /* entre 1 et 100 */
            p->provisions[i] += val;
            p->commandes[j] += val;
        }
    }

    /* Génération de la matrice de coûts. */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            p->couts[i][j] = (rand() % 100) + 1;
    }

    return p;
}

void detruire_probleme(Probleme *p)
{
    if (!p)
        return;

    if (p->couts) {
        for (int i = 0; i < p->nb_fournisseurs; i++)
            free(p->couts[i]);
        free(p->couts);
    }

    free(p->provisions);
    free(p->commandes);
    free(p);
}

/*
 * Potentiels et coûts marginaux (versions silencieuses et dynamiques) -------
 */

static int choisir_sommet_depart_dyn(const Base *b, int total, int n)
{
    int *degres = calloc((size_t)total, sizeof(int));
    if (!degres)
        return 0;

    for (int k = 0; k < b->nb_arcs; k++) {
        int fi = b->arcs[k][0];
        int cj = b->arcs[k][1];
        int u = fi;
        int v = n + cj;

        if (u >= 0 && u < total)
            degres[u]++;
        if (v >= 0 && v < total)
            degres[v]++;
    }

    int sommet = 0;
    int deg_max = -1;
    for (int i = 0; i < total; i++) {
        if (degres[i] > deg_max) {
            deg_max = degres[i];
            sommet = i;
        }
    }

    free(degres);
    return sommet;
}

static void calculer_potentiels_dyn(const Probleme *p, const Base *b,
                                     int *pot_f, int *pot_c)
{
    int n = p->nb_fournisseurs;
    int m = p->nb_clients;
    int total = n + m;

    if (!b || total <= 0)
        return;

    int *pot_sommet = calloc((size_t)total, sizeof(int));
    int *visite = calloc((size_t)total, sizeof(int));
    int *file = malloc((size_t)total * sizeof(int));

    if (!pot_sommet || !visite || !file) {
        free(pot_sommet);
        free(visite);
        free(file);
        return;
    }

    int racine = choisir_sommet_depart_dyn(b, total, n);
    int tete = 0, queue = 0;
    file[queue++] = racine;
    visite[racine] = 1;
    pot_sommet[racine] = 0;

    while (tete < queue) {
        int u = file[tete++];
        int est_fournisseur = (u < n);

        for (int i = 0; i < b->nb_arcs; i++) {
            int fi = b->arcs[i][0];
            int cj = b->arcs[i][1];
            int noeud_f = fi;
            int noeud_c = n + cj;
            int cout = p->couts[fi][cj];

            if (est_fournisseur && u == noeud_f) {
                int v = noeud_c;
                if (!visite[v]) {
                    pot_sommet[v] = pot_sommet[u] - cout;
                    visite[v] = 1;
                    file[queue++] = v;
                }
            } else if (!est_fournisseur && u == noeud_c) {
                int v = noeud_f;
                if (!visite[v]) {
                    pot_sommet[v] = cout + pot_sommet[u];
                    visite[v] = 1;
                    file[queue++] = v;
                }
            }
        }
    }

    for (int i = 0; i < n; i++)
        pot_f[i] = pot_sommet[i];

    for (int j = 0; j < m; j++)
        pot_c[j] = pot_sommet[n + j];

    free(pot_sommet);
    free(visite);
    free(file);
}

static int arc_dans_base_simple(const Base *b, int i, int j)
{
    for (int k = 0; k < b->nb_arcs; k++) {
        if (b->arcs[k][0] == i && b->arcs[k][1] == j)
            return 1;
    }
    return 0;
}

/* Renvoie 1 si optimal, 0 sinon (avec i_entree/j_entree renseignés). */
static int trouver_arc_ameliorant(const Probleme *p, const Base *b,
                                  const int *pot_f, const int *pot_c,
                                  int *i_entree, int *j_entree)
{
    int n = p->nb_fournisseurs;
    int m = p->nb_clients;

    int meilleur_i = -1;
    int meilleur_j = -1;
    int meilleur_marginal = 0;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (arc_dans_base_simple(b, i, j))
                continue;

            int cp = pot_f[i] - pot_c[j];
            int marginal = p->couts[i][j] - cp;

            if (marginal < meilleur_marginal) {
                meilleur_marginal = marginal;
                meilleur_i = i;
                meilleur_j = j;
            }
        }
    }

    if (meilleur_i == -1)
        return 1; /* optimale */

    if (i_entree)
        *i_entree = meilleur_i;
    if (j_entree)
        *j_entree = meilleur_j;

    return 0;
}

/*
 * Marche-pied (version silencieuse) ----------------------------------------
 */

static int trouver_chemin_dyn(const Base *b, int n, int m, int source, int cible, int *parent)
{
    int total = n + m;
    int *file = malloc((size_t)total * sizeof(int));
    if (!file)
        return 0;

    for (int i = 0; i < total; i++)
        parent[i] = -1;

    int tete = 0, queue = 0;
    file[queue++] = source;
    parent[source] = source;

    while (tete < queue && parent[cible] == -1) {
        int u = file[tete++];

        for (int k = 0; k < b->nb_arcs; k++) {
            int fi = b->arcs[k][0];
            int cj = b->arcs[k][1];

            int a = fi;
            int c = n + cj;
            int v = -1;

            if (u == a)
                v = c;
            else if (u == c)
                v = a;

            if (v >= 0 && parent[v] == -1) {
                parent[v] = u;
                file[queue++] = v;
            }
        }
    }

    free(file);
    return parent[cible] != -1;
}

static int marche_pied_silencieux(const Base *b, Solution *s,
                                  int i_entree, int j_entree,
                                  int *i_sortie_ptr, int *j_sortie_ptr)
{
    int n = s->nb_fournisseurs;
    int m = s->nb_clients;
    int total = n + m;

    int *parent = malloc((size_t)total * sizeof(int));
    int *chemin_noeuds = malloc((size_t)total * sizeof(int));
    int *cycle_i = NULL;
    int *cycle_j = NULL;
    int *signe = NULL;

    if (!parent || !chemin_noeuds) {
        free(parent);
        free(chemin_noeuds);
        return -1;
    }

    int noeud_f = i_entree;
    int noeud_c = n + j_entree;

    if (!trouver_chemin_dyn(b, n, m, noeud_f, noeud_c, parent)) {
        free(parent);
        free(chemin_noeuds);
        if (i_sortie_ptr)
            *i_sortie_ptr = -1;
        if (j_sortie_ptr)
            *j_sortie_ptr = -1;
        return -1;
    }

    int len = 0;
    int cur = noeud_c;
    while (1) {
        chemin_noeuds[len++] = cur;
        if (cur == noeud_f || len > total)
            break;
        cur = parent[cur];
    }

    for (int i = 0; i < len / 2; i++) {
        int tmp = chemin_noeuds[i];
        chemin_noeuds[i] = chemin_noeuds[len - 1 - i];
        chemin_noeuds[len - 1 - i] = tmp;
    }

    int cycle_taille = len;
    cycle_i = malloc((size_t)cycle_taille * sizeof(int));
    cycle_j = malloc((size_t)cycle_taille * sizeof(int));
    signe = malloc((size_t)cycle_taille * sizeof(int));

    if (!cycle_i || !cycle_j || !signe) {
        free(parent);
        free(chemin_noeuds);
        free(cycle_i);
        free(cycle_j);
        free(signe);
        return -1;
    }

    cycle_i[0] = i_entree;
    cycle_j[0] = j_entree;

    for (int e = 0; e < len - 1; e++) {
        int u = chemin_noeuds[e];
        int v = chemin_noeuds[e + 1];
        int fi, cj;

        if (u < n && v >= n) {
            fi = u;
            cj = v - n;
        } else if (v < n && u >= n) {
            fi = v;
            cj = u - n;
        } else {
            free(parent);
            free(chemin_noeuds);
            free(cycle_i);
            free(cycle_j);
            free(signe);
            return -1;
        }

        cycle_i[e + 1] = fi;
        cycle_j[e + 1] = cj;
    }

    for (int e = 0; e < cycle_taille; e++)
        signe[e] = (e % 2 == 0) ? 1 : -1;

    int theta = INT_MAX;
    for (int e = 0; e < cycle_taille; e++) {
        if (signe[e] == -1) {
            int x = s->x[cycle_i[e]][cycle_j[e]];
            if (x < theta)
                theta = x;
        }
    }

    if (theta == INT_MAX)
        theta = 0;

    for (int e = 0; e < cycle_taille; e++)
        s->x[cycle_i[e]][cycle_j[e]] += signe[e] * theta;

    int i_sortie = -1;
    int j_sortie = -1;
    for (int e = 0; e < cycle_taille; e++) {
        if (signe[e] == -1 && s->x[cycle_i[e]][cycle_j[e]] == 0) {
            i_sortie = cycle_i[e];
            j_sortie = cycle_j[e];
            break;
        }
    }

    if (i_sortie_ptr)
        *i_sortie_ptr = i_sortie;
    if (j_sortie_ptr)
        *j_sortie_ptr = j_sortie;

    free(parent);
    free(chemin_noeuds);
    free(cycle_i);
    free(cycle_j);
    free(signe);

    return theta;
}

static double optimiser_par_marche_pied(const Probleme *p, Solution *s)
{
    int n = p->nb_fournisseurs;
    int m = p->nb_clients;
    int *pot_f = malloc((size_t)n * sizeof(int));
    int *pot_c = malloc((size_t)m * sizeof(int));

    if (!pot_f || !pot_c) {
        free(pot_f);
        free(pot_c);
        return -1.0;
    }

    Base *b = construire_base(s);
    if (!b) {
        free(pot_f);
        free(pot_c);
        return -1.0;
    }

    clock_t start = clock();

    int iteration = 0;
    int optimal = 0;
    const int iteration_max = n * m * 2; /* borne de sécurité */

    while (!optimal && iteration < iteration_max) {
        iteration++;

        if (!base_est_arbre(b, n, m)) {
            int saved_fd = -1;
            silence_stdout(&saved_fd);
            Base *corrigee = corriger_base(b, s, n, m);
            restaurer_stdout(saved_fd);
            liberer_base(b);
            b = corrigee;
        }

        calculer_potentiels_dyn(p, b, pot_f, pot_c);

        int i_entree = -1;
        int j_entree = -1;
        optimal = trouver_arc_ameliorant(p, b, pot_f, pot_c, &i_entree, &j_entree);
        if (optimal)
            break;

        int i_sortie = -1;
        int j_sortie = -1;
        int theta = marche_pied_silencieux(b, s, i_entree, j_entree, &i_sortie, &j_sortie);

        if (theta < 0)
            break;

        int remplace = 0;
        if (i_sortie >= 0 && j_sortie >= 0) {
            for (int k = 0; k < b->nb_arcs; k++) {
                if (b->arcs[k][0] == i_sortie && b->arcs[k][1] == j_sortie) {
                    b->arcs[k][0] = i_entree;
                    b->arcs[k][1] = j_entree;
                    remplace = 1;
                    break;
                }
            }
        }

        if (!remplace) {
            liberer_base(b);
            b = construire_base(s);
        }
    }

    clock_t end = clock();

    liberer_base(b);
    free(pot_f);
    free(pot_c);

    return (double)(end - start) / CLOCKS_PER_SEC;
}

/*
 * Mesure de temps des solutions initiales ----------------------------------
 */

double mesurer_temps_nord_ouest(const Probleme *p, Solution **solution_out)
{
    if (!p)
        return -1.0;

    Solution *s = creer_solution_vide(p);
    if (!s)
        return -1.0;

    int saved_fd = -1;
    silence_stdout(&saved_fd);

    clock_t start = clock();
    coin_nord_ouest(p, s);
    clock_t end = clock();

    restaurer_stdout(saved_fd);

    if (solution_out)
        *solution_out = s;
    else
        liberer_solution(s);

    return (double)(end - start) / CLOCKS_PER_SEC;
}

double mesurer_temps_balas_hammer(const Probleme *p, Solution **solution_out)
{
    if (!p)
        return -1.0;

    Solution *s = creer_solution_vide(p);
    if (!s)
        return -1.0;

    int saved_fd = -1;
    silence_stdout(&saved_fd);

    clock_t start = clock();
    balas_hammer(p, s);
    clock_t end = clock();

    restaurer_stdout(saved_fd);

    if (solution_out)
        *solution_out = s;
    else
        liberer_solution(s);

    return (double)(end - start) / CLOCKS_PER_SEC;
}

/*
 * Boucle d'expérimentation --------------------------------------------------
 */

void lancer_etude_complexite(void)
{
    int tailles[] = {10, 40, 100, 200, 400, 1000 };
    int nb_tailles = (int)(sizeof(tailles) / sizeof(tailles[0]));
    int repetitions = 100;

    FILE *csv = fopen("resultats_complexite.csv", "w");
    if (!csv) {
        fprintf(stderr, "Impossible d'ouvrir resultats_complexite.csv en écriture\n");
        return;
    }

    fprintf(csv, "n;iteration;theta_no;theta_bh;t_no;t_bh;total_no;total_bh;ratio\n");

    for (int idx = 0; idx < nb_tailles; idx++) {
        int n = tailles[idx];
        printf("Taille %d...\n", n);

        for (int k = 0; k < repetitions; k++) {
            Probleme *p = generer_probleme_aleatoire(n);
            if (!p) {
                fprintf(stderr, "Generation du probleme (%d) impossible\n", n);
                continue;
            }

            Solution *s_no = NULL;
            Solution *s_bh = NULL;

            double theta_no = mesurer_temps_nord_ouest(p, &s_no);
            double theta_bh = mesurer_temps_balas_hammer(p, &s_bh);

            double t_no = (s_no) ? optimiser_par_marche_pied(p, s_no) : -1.0;
            double t_bh = (s_bh) ? optimiser_par_marche_pied(p, s_bh) : -1.0;

            double total_no = (theta_no >= 0 && t_no >= 0) ? theta_no + t_no : -1.0;
            double total_bh = (theta_bh >= 0 && t_bh >= 0) ? theta_bh + t_bh : -1.0;
            double ratio = (total_bh > 0) ? (total_no / total_bh) : 0.0;

            fprintf(csv, "%d;%d;%.6f;%.6f;%.6f;%.6f;%.6f;%.6f;%.6f\n",
                    n, k,
                    theta_no, theta_bh,
                    t_no, t_bh,
                    total_no, total_bh,
                    ratio);

            if (s_no)
                liberer_solution(s_no);
            if (s_bh)
                liberer_solution(s_bh);
            detruire_probleme(p);

            if ((k + 1) % 10 == 0)
                printf("  Iteration %d/%d terminee\n", k + 1, repetitions);
        }
    }

    fclose(csv);
    printf("Etude terminee. Resultats dans resultats_complexite.csv\n");
}

int main(void)
{
    srand((unsigned int)time(NULL));
    lancer_etude_complexite();
    return 0;
}