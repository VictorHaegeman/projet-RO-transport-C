#include <stdio.h>
#include <string.h>
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

    if (strcmp(methode, "no") == 0) {
        printf("\n=== MÉTHODE : NORD-OUEST ===\n");
        coin_nord_ouest(p, s);
    }
    else if (strcmp(methode, "bh") == 0) {
        printf("\n=== MÉTHODE : BALAS-HAMMER ===\n");
        balas_hammer(p, s);
    }
    else {
        printf("Méthode inconnue (utiliser 'no' ou 'bh').\n");
        liberer_probleme(p);
        liberer_solution(s);
        return 1;
    }

    afficher_solution(p, s);

    // === Construction et affichage de la base ===
    Base *b = construire_base(s);
    afficher_base_liste(b);  // debug textuel
    afficher_base_graphe(b, p->nb_fournisseurs, p->nb_clients);  // dessin style S/T

    liberer_base(b);
    liberer_probleme(p);
    liberer_solution(s);

    return 0;
}
