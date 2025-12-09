#include <stdio.h>
#include <string.h>
#include <time.h>
#include "problem.h"
#include "nord_ouest.h"
#include "balas_hammer.h"
#include "potentiel.h"
#include "marche_pied.h"
#include "base.h"

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("Usage : %s <fichier.txt> <no|bh>\n", argv[0]);
        return 1;
    }

    const char *fichier = argv[1];
    const char *methode = argv[2];

    Probleme *p = lire_probleme(fichier);
    if (!p) {
        printf("Erreur : impossible de lire le fichier.\n");
        return 1;
    }

    afficher_probleme(p);

    Solution *s = creer_solution_vide(p);

    clock_t start;
    clock_t end;

    if (strcmp(methode, "no") == 0) {
        printf("\n=== MÉTHODE : NORD-OUEST ===\n");
        start = clock();
        coin_nord_ouest(p, s);
        end = clock();
    }
    else if (strcmp(methode, "bh") == 0) {
        printf("\n=== MÉTHODE : BALAS-HAMMER ===\n");
        start = clock();
        balas_hammer(p, s);
        end = clock();
    }
    else {
        printf("Méthode inconnue (utiliser 'no' ou 'bh').\n");
        liberer_probleme(p);
        liberer_solution(s);
        return 1;
    }

    double time_spent_methode = (double)(end - start) / CLOCKS_PER_SEC;

    printf("\n=== SOLUTION DE DÉPART ===\n");
    afficher_solution(p, s);

    // === Construction initiale de la base ===
    Base *b = construire_base(s);

    //  Potentiels + coûts marginaux + marche-pied (boucle complète)
    int pot_f[p->nb_fournisseurs];
    int pot_c[p->nb_clients];

    int i_entree, j_entree;
    int i_sortie, j_sortie;
    int optimal = 0;
    int iteration = 1;
    double time_spent_mp=0.0;

    // c'est pas bon ici je pense 
    while (1) {
        printf("\n================== ITERATION %d ==================\n", iteration++);

        // (Re)affichage de la base courante
        afficher_base_liste(b);  // debug textuel
        afficher_base_graphe(b, p->nb_fournisseurs, p->nb_clients);  // dessin style S/T

        // Vérification/correction de la base à chaque itération pour que les potentiels soient cohérents
        if (base_est_arbre(b, p->nb_fournisseurs, p->nb_clients)) {
            printf("\n>>> La base est un arbre. OK pour les potentiels.\n");
        } else {
            printf("\n>>> La base n'est PAS un arbre ! Correction nécessaire.\n");
            printf("\n>>> Correction automatique de la base...\n");

            Base *b2 = corriger_base(b, s,
                                     p->nb_fournisseurs,
                                     p->nb_clients);

            liberer_base(b);
            b = b2;

            printf("\n--- Nouvelle base après correction ---\n");
            afficher_base_liste(b);
            afficher_base_graphe(b,
                                 p->nb_fournisseurs,
                                 p->nb_clients);
        }

        // 1) Potentiels
        calculer_potentiels(p, b, pot_f, pot_c);
        afficher_potentiels(p, pot_f, pot_c);
        afficher_table_couts_potentiels(p, pot_f, pot_c);

        // 2) Coûts marginaux + choix de l'arête améliorante
        optimal = calculer_et_afficher_couts_marginaux(p, s, b,
                                                       pot_f, pot_c,
                                                       &i_entree, &j_entree);

        

        // 3) Marche-pied si ce n'est pas optimal
        if (!optimal) {
            printf("\n=== MARCHE-PIED SUR L'ARÊTE AMÉLIORANTE (F%d, C%d) ===\n",
                   i_entree, j_entree);
            start = clock();
            i_sortie = -1;
            j_sortie = -1;
            int theta_mp = marche_pied(b, s, i_entree, j_entree, &i_sortie, &j_sortie);
            end = clock();

            printf("\n=== NOUVELLE SOLUTION APRÈS MARCHE-PIED ===\n");
            afficher_solution(p, s);

            time_spent_mp += (double)(end - start) / CLOCKS_PER_SEC;

            if (theta_mp == 0) {
                printf("Pivot dégénéré (theta = 0) : mise à jour de la base pour changer de proposition.\n");
            }

            // Mettre à jour la base en remplaçant l'arc sortant par l'arc entrant,
            // même en cas de pivot dégénéré (theta = 0) pour éviter de boucler.
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
                printf("Avertissement : base non mise à jour (arc sortant introuvable). Reconstruction.\n");
                liberer_base(b);
                b = construire_base(s);
            }
        } else {
            printf("\n=== SOLUTION DÉJÀ OPTIMALE, PAS DE MARCHE-PIED ===\n");
            break;
        }

    }

    printf("\n=== Meusure du temps ===\n");
    printf("Temps méthode initiale (%s) : %.6f secondes\n", methode, time_spent_methode);
    printf("Temps total marche-pied : %.6f secondes\n", time_spent_mp);
    printf("=============================================\n");

    liberer_base(b);
    liberer_probleme(p);
    liberer_solution(s);

    return 0;
}
