import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# ---------------------------------------------------------
# Charger le CSV produit par ./complexite
# ---------------------------------------------------------

df = pd.read_csv("../resultats_complexite.csv", sep=";")

# Garder uniquement les tailles voulues
df = df[df["n"].isin([10, 40, 100])]

# ---------------------------------------------------------
# Choix de la métrique à visualiser
# (total_no, total_bh, ratio, theta_no, t_no, etc.)
# ---------------------------------------------------------

metric = "total_no"   # <<< change ici pour afficher une autre colonne

# ---------------------------------------------------------
# Amélioration de la lisibilité : ajout d'un jitter horizontal
# ---------------------------------------------------------

np.random.seed(42)   # pour un jitter reproductible
df["n_jitter"] = df["n"] + np.random.uniform(-1, 1, size=len(df))

# ---------------------------------------------------------
# Création du nuage de points
# ---------------------------------------------------------

plt.figure(figsize=(12, 6))

sns.scatterplot(
    data=df,
    x="n_jitter",        # X avec jitter pour séparer les points
    y=metric,
    hue="n",
    palette="viridis",
    s=80,
    alpha=0.8,
    edgecolor="black"
)

# ---------------------------------------------------------
# Mise en forme du graphe
# ---------------------------------------------------------

plt.title(f"Nuage de points – Complexité expérimentale ({metric})", fontsize=16)
plt.xlabel("Taille du problème n", fontsize=14)
plt.ylabel(f"Valeur mesurée ({metric})", fontsize=14)

# On force les ticks aux vraies valeurs 10, 40, 100
plt.xticks([10, 40, 100])

plt.grid(True, linestyle="--", alpha=0.3)

plt.legend(title="n", fontsize=12, title_fontsize=13)

plt.tight_layout()
plt.show()
