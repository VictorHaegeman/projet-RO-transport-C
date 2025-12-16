import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# ---- Charger le CSV ----
df = pd.read_csv("../resultats_complexite.csv", sep=";")

sizes = [10, 40, 100, 400]
df = df[df["n"].isin(sizes)].copy()

# (optionnel) garder seulement les lignes valides
df = df[(df["total_no"] >= 0) & (df["total_bh"] >= 0)]

np.random.seed(42)

fig, axes = plt.subplots(2, 2, figsize=(14, 8))
axes = axes.ravel()

for ax, n in zip(axes, sizes):
    sub = df[df["n"] == n]

    # 2 colonnes sur l'axe X : 0 = NW, 1 = BH (avec jitter)
    x_no = 0 + np.random.uniform(-0.08, 0.08, size=len(sub))
    x_bh = 1 + np.random.uniform(-0.08, 0.08, size=len(sub))

    ax.scatter(x_no, sub["total_no"], s=35, alpha=0.8, label="Nord-Ouest (total_no)")
    ax.scatter(x_bh, sub["total_bh"], s=35, alpha=0.8, label="Balas-Hammer (total_bh)")

    ax.set_title(f"n = {n} (100 exécutions)")
    ax.set_xticks([0, 1])
    ax.set_xticklabels(["Nord-Ouest", "Balas-Hammer"])
    ax.set_ylabel("Temps (s)")
    ax.grid(True, linestyle="--", alpha=0.3)

# Une seule légende globale (évite de répéter 4 fois)
handles, labels = axes[0].get_legend_handles_labels()
fig.legend(handles, labels, loc="upper center", ncol=2)

fig.suptitle("Nuages de points par taille n (total_no vs total_bh)", y=0.98, fontsize=14)
plt.tight_layout(rect=[0, 0, 1, 0.94])
plt.show()
