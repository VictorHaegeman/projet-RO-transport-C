#include <stdio.h>
#include "nord_ouest.h"
#include "trace.h"

void coin_nord_ouest(const Probleme *p, Solution *s)
{
    int provisions[p->nb_fournisseurs];
    int commandes[p->nb_clients];

    for (int i = 0; i < p->nb_fournisseurs; i++)
        provisions[i] = p->provisions[i];

    for (int j = 0; j < p->nb_clients; j++)
        commandes[j] = p->commandes[j];

    int i = 0, j = 0;

    trace("\n--- Déroulé de la méthode du Coin Nord-Ouest ---\n");

    while (i < p->nb_fournisseurs && j < p->nb_clients) {

        int q = (provisions[i] < commandes[j]) ? provisions[i] : commandes[j];
        s->x[i][j] = q;

        trace("Case (%d,%d) ← %d\n", i, j, q);

        provisions[i] -= q;
        commandes[j] -= q;

        if (provisions[i] == 0 && i + 1 < p->nb_fournisseurs) i++;
        else if (commandes[j] == 0 && j + 1 < p->nb_clients) j++;
        else {
            if (provisions[i] == 0) i++;
            if (commandes[j] == 0) j++;
        }
    }

    trace("--- Fin Nord-Ouest ---\n\n");
}