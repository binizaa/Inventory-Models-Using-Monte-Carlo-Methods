"""Paso 6 - Graficas de la simulacion.

Lee los CSV que exporta el programa en C++ (carpeta resultados/) y genera:
  1. demanda_diaria.png   Demanda diaria simulada por ubicacion
  2. inventario.png       Evolucion del inventario con punto de reorden e inventario de seguridad
  3. pedidos.png          Fechas de emision y recepcion de pedidos
  4. comparacion.png      Costos, nivel de servicio y faltantes entre las tres ciudades

Uso:  .venv/bin/python graficas.py
"""

from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
import pandas as pd

RESULTS = Path("resultados")

# Paleta categorica (una por ciudad) y tinta neutra para texto y referencias
CITY_COLORS = ["#2a78d6", "#eb6834", "#1baf7a"]
SURFACE = "#fcfcfb"
TEXT = "#0b0b0b"
TEXT_2 = "#52514e"
GRID = "#e4e3df"
REFERENCE = "#8a8984"

plt.rcParams.update({
    "figure.facecolor": SURFACE,
    "axes.facecolor": SURFACE,
    "axes.edgecolor": GRID,
    "axes.labelcolor": TEXT_2,
    "axes.titlecolor": TEXT,
    "axes.titleweight": "bold",
    "axes.titlesize": 11,
    "axes.titlelocation": "left",
    "axes.grid": True,
    "axes.grid.axis": "y",
    "axes.axisbelow": True,
    "grid.color": GRID,
    "grid.linewidth": 0.8,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "xtick.color": TEXT_2,
    "ytick.color": TEXT_2,
    "font.size": 9,
    "legend.frameon": False,
    "savefig.dpi": 150,
    "savefig.bbox": "tight",
})


def load():
    daily = pd.read_csv(RESULTS / "registro_diario.csv")
    orders = pd.read_csv(RESULTS / "pedidos.csv")
    summary = pd.read_csv(RESULTS / "indicadores.csv")
    cities = list(dict.fromkeys(daily["ciudad"]))
    colors = dict(zip(cities, CITY_COLORS))
    return daily, orders, summary, cities, colors


def demanda_diaria(daily, cities, colors):
    fig, axes = plt.subplots(len(cities), 1, figsize=(9, 2.3 * len(cities)), sharex=True, sharey=True)
    for ax, city in zip(axes, cities):
        d = daily[daily["ciudad"] == city]
        ax.bar(d["dia"], d["demanda"], width=0.7, color=colors[city])
        mean = d["demanda"].mean()
        ax.axhline(mean, color=REFERENCE, linewidth=1, linestyle="--")
        ax.text(d["dia"].max() + 0.6, mean, f"promedio {mean:.1f}", color=TEXT_2, va="center", fontsize=8)
        ax.set_title(city)
        ax.set_ylabel("Unidades")
    axes[-1].set_xlabel("Dia")
    fig.suptitle("Demanda diaria simulada por ubicacion", x=0.01, ha="left", fontweight="bold", color=TEXT)
    fig.tight_layout()
    fig.savefig(RESULTS / "demanda_diaria.png")
    plt.close(fig)


def inventario(daily, orders, cities, colors):
    fig, axes = plt.subplots(len(cities), 1, figsize=(9, 2.8 * len(cities)), sharex=True)
    for ax, city in zip(axes, cities):
        d = daily[daily["ciudad"] == city]
        o = orders[orders["ciudad"] == city]
        rop, ss = d["rop"].iloc[0], d["ss"].iloc[0]
        last_day = d["dia"].max()

        ax.axhspan(0, ss, color=REFERENCE, alpha=0.15, linewidth=0)
        ax.axhline(rop, color=REFERENCE, linewidth=1, linestyle="--")
        ax.text(last_day + 0.6, rop, f"ROP = {rop}", color=TEXT_2, va="center", fontsize=8)
        ax.text(last_day + 0.6, ss / 2, f"SS = {ss}", color=TEXT_2, va="center", fontsize=8)

        ax.plot(d["dia"], d["inv_final"], color=colors[city], linewidth=2, label="Inventario final")

        issued = d[d["dia"].isin(o["emision"])]
        ax.plot(issued["dia"], issued["inv_final"], linestyle="none", marker="^", markersize=8,
                color=colors[city], markeredgecolor=SURFACE, markeredgewidth=1.5, label="Pedido emitido")
        received = d[d["dia"].isin(o["recepcion"])]
        ax.plot(received["dia"], received["inv_final"], linestyle="none", marker="o", markersize=8,
                color=SURFACE, markeredgecolor=colors[city], markeredgewidth=2, label="Pedido recibido")

        ax.set_title(city)
        ax.set_ylabel("Unidades")
        ax.set_ylim(bottom=0)
    axes[-1].set_xlabel("Dia")
    fig.suptitle("Evolucion del inventario, punto de reorden (ROP) e inventario de seguridad (SS)",
                 x=0.01, ha="left", fontweight="bold", color=TEXT)
    fig.tight_layout(rect=(0, 0, 1, 0.97))
    # Leyenda neutra: la ciudad ya la identifica el titulo de cada panel
    handles = [
        Line2D([], [], color=TEXT_2, linewidth=2, label="Inventario final"),
        Line2D([], [], color=TEXT_2, linestyle="none", marker="^", markersize=8, label="Pedido emitido"),
        Line2D([], [], color=SURFACE, linestyle="none", marker="o", markersize=8,
               markeredgecolor=TEXT_2, markeredgewidth=2, label="Pedido recibido"),
        Line2D([], [], color=REFERENCE, linewidth=1, linestyle="--", label="Punto de reorden"),
        Patch(color=REFERENCE, alpha=0.15, label="Inventario de seguridad"),
    ]
    fig.legend(handles=handles, loc="upper left", bbox_to_anchor=(0.01, 0.965), ncol=5, fontsize=8)
    fig.subplots_adjust(top=0.89)
    fig.savefig(RESULTS / "inventario.png")
    plt.close(fig)


def pedidos(daily, orders, cities, colors):
    horizon = daily["dia"].max()
    fig, ax = plt.subplots(figsize=(9, 0.8 + 0.7 * len(cities)))
    for row, city in enumerate(cities):
        for _, o in orders[orders["ciudad"] == city].iterrows():
            ax.plot([o["emision"], o["recepcion"]], [row, row], color=colors[city], linewidth=2)
            ax.plot(o["emision"], row, marker="^", markersize=8, color=colors[city],
                    markeredgecolor=SURFACE, markeredgewidth=1.5)
            ax.plot(o["recepcion"], row, marker="o", markersize=8, color=SURFACE,
                    markeredgecolor=colors[city], markeredgewidth=2)
            ax.text((o["emision"] + o["recepcion"]) / 2, row + 0.18, f"{o['cantidad']} u.",
                    ha="center", color=TEXT_2, fontsize=8)
    ax.axvline(horizon, color=REFERENCE, linewidth=1, linestyle=":")
    ax.text(horizon, len(cities) - 0.45, " fin del horizonte", color=TEXT_2, fontsize=8)
    ax.set_yticks(range(len(cities)), cities)
    ax.set_ylim(-0.6, len(cities) - 0.3)
    ax.invert_yaxis()
    ax.set_xlim(0, max(horizon, orders["recepcion"].max() if len(orders) else horizon) + 1)
    ax.grid(axis="x")
    ax.grid(axis="y", visible=False)
    ax.set_xlabel("Dia   (triangulo = emision, circulo = recepcion)")
    ax.set_title("Fechas de emision y recepcion de pedidos")
    fig.tight_layout()
    fig.savefig(RESULTS / "pedidos.png")
    plt.close(fig)


def comparacion(summary, cities, colors):
    s = summary.set_index("ciudad").loc[cities]
    x = range(len(cities))
    bar_colors = [colors[c] for c in cities]
    fig, axes = plt.subplots(1, 3, figsize=(11, 3.6))

    ax = axes[0]
    ax.bar(x, s["costo_ordenar"], width=0.6, color=bar_colors, edgecolor=SURFACE, linewidth=2)
    ax.bar(x, s["costo_mantener"], width=0.6, bottom=s["costo_ordenar"], color=bar_colors,
           hatch="///", alpha=0.55, edgecolor=SURFACE, linewidth=2)
    for i, total in enumerate(s["costo_total"]):
        ax.text(i, total, f"${total:,.0f}", ha="center", va="bottom", color=TEXT, fontsize=8)
    ax.set_title("Costo total simulado ($)")
    ax.text(0, -0.2, "solido = ordenar, rayado = mantener", transform=ax.transAxes, color=TEXT_2, fontsize=8)

    ax = axes[1]
    ax.bar(x, s["nivel_servicio"] * 100, width=0.6, color=bar_colors)
    for i, v in enumerate(s["nivel_servicio"] * 100):
        ax.text(i, v, f"{v:.1f}%", ha="center", va="bottom", color=TEXT, fontsize=8)
    ax.set_ylim(0, 105)
    ax.set_title("Nivel de servicio por unidades (%)")

    ax = axes[2]
    ax.bar(x, s["unidades_no_atendidas"], width=0.6, color=bar_colors)
    for i, (u, dias) in enumerate(zip(s["unidades_no_atendidas"], s["dias_faltante"])):
        ax.text(i, u, f"{u} u. / {dias} dias", ha="center", va="bottom", color=TEXT, fontsize=8)
    ax.set_ylim(0, max(1, s["unidades_no_atendidas"].max()) * 1.15)
    ax.set_title("Unidades no atendidas (faltantes)")

    for ax in axes:
        ax.set_xticks(list(x), cities, fontsize=8)
    fig.suptitle("Comparacion entre ciudades", x=0.01, ha="left", fontweight="bold", color=TEXT)
    fig.tight_layout()
    fig.savefig(RESULTS / "comparacion.png")
    plt.close(fig)


def main():
    if not (RESULTS / "registro_diario.csv").exists():
        print("No se encontro resultados/registro_diario.csv; corre primero ./main")
        return
    daily, orders, summary, cities, colors = load()
    demanda_diaria(daily, cities, colors)
    inventario(daily, orders, cities, colors)
    pedidos(daily, orders, cities, colors)
    comparacion(summary, cities, colors)
    print(f"Graficas guardadas en {RESULTS}/: demanda_diaria.png, inventario.png, pedidos.png, comparacion.png")


if __name__ == "__main__":
    main()
