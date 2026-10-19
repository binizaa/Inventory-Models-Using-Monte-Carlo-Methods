"""Paso 6 - Graficas de la simulacion.

Lee los CSV que exporta el programa en C++ (carpeta resultados/) y genera:
  1. demanda_diaria.png   Demanda diaria simulada por ubicacion
  2. inventario.png       Evolucion del inventario con punto de reorden e inventario de seguridad
  3. pedidos.png          Fechas de emision y recepcion de pedidos
  4. comparacion.png      Costos, nivel de servicio y faltantes entre las tres ciudades

Uso:  .venv/bin/python graficas.py

Las funciones devuelven la figura sin guardarla, asi la interfaz (interfaz.py)
puede mostrar las mismas graficas dentro de la ventana.
"""

from pathlib import Path

import matplotlib
from matplotlib.figure import Figure
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
import pandas as pd

RESULTS = Path(__file__).resolve().parent / "resultados"

# Paleta categorica (una por ciudad) y tinta neutra para texto y referencias
CITY_COLORS = ["#2a78d6", "#eb6834", "#1baf7a"]
SURFACE = "#fcfcfb"
TEXT = "#0b0b0b"
TEXT_2 = "#52514e"
GRID = "#e4e3df"
REFERENCE = "#8a8984"

matplotlib.rcParams.update({
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


def load(results=RESULTS):
    daily = pd.read_csv(results / "registro_diario.csv")
    orders = pd.read_csv(results / "pedidos.csv")
    summary = pd.read_csv(results / "indicadores.csv")
    cities = list(dict.fromkeys(daily["ciudad"]))
    colors = dict(zip(cities, CITY_COLORS))
    return daily, orders, summary, cities, colors


def _suptitle(fig, text):
    fig.suptitle(text, x=0.01, ha="left", fontweight="bold", color=TEXT)


def demanda_diaria(daily, cities, colors):
    fig = Figure(figsize=(9, 2.3 * len(cities)), layout="constrained")
    axes = fig.subplots(len(cities), 1, sharex=True, sharey=True, squeeze=False)[:, 0]
    last_day = daily["dia"].max()
    for ax, city in zip(axes, cities):
        d = daily[daily["ciudad"] == city]
        ax.bar(d["dia"], d["demanda"], width=0.7, color=colors[city])
        mean = d["demanda"].mean()
        ax.axhline(mean, color=REFERENCE, linewidth=1, linestyle="--")
        ax.text(last_day + 0.8, mean, f"promedio {mean:.1f}", color=TEXT_2, va="center", fontsize=8)
        ax.set_xlim(0, last_day + 6)
        ax.set_title(city)
        ax.set_ylabel("Unidades")
    axes[-1].set_xlabel("Dia")
    _suptitle(fig, "Demanda diaria simulada por ubicacion")
    return fig


def inventario(daily, orders, cities, colors):
    fig = Figure(figsize=(9, 2.8 * len(cities)), layout="constrained")
    fig.get_layout_engine().set(rect=(0, 0, 1, 0.78))
    axes = fig.subplots(len(cities), 1, sharex=True, squeeze=False)[:, 0]
    last_day = daily["dia"].max()
    for ax, city in zip(axes, cities):
        d = daily[daily["ciudad"] == city]
        o = orders[orders["ciudad"] == city]
        rop, ss = d["rop"].iloc[0], d["ss"].iloc[0]

        ax.axhspan(0, ss, color=REFERENCE, alpha=0.15, linewidth=0)
        ax.axhline(rop, color=REFERENCE, linewidth=1, linestyle="--")
        ax.text(last_day + 0.8, rop, f"ROP = {rop}", color=TEXT_2, va="center", fontsize=8)
        ax.text(last_day + 0.8, ss / 2, f"SS = {ss}", color=TEXT_2, va="center", fontsize=8)

        ax.plot(d["dia"], d["inv_final"], color=colors[city], linewidth=2)

        issued = d[d["dia"].isin(o["emision"])]
        ax.plot(issued["dia"], issued["inv_final"], linestyle="none", marker="^", markersize=8,
                color=colors[city], markeredgecolor=SURFACE, markeredgewidth=1.5)
        received = d[d["dia"].isin(o["recepcion"])]
        ax.plot(received["dia"], received["inv_final"], linestyle="none", marker="o", markersize=8,
                color=SURFACE, markeredgecolor=colors[city], markeredgewidth=2)

        ax.set_xlim(0, last_day + 6)
        ax.set_ylim(bottom=0)
        ax.set_title(city)
        ax.set_ylabel("Unidades")
    axes[-1].set_xlabel("Dia")
    _suptitle(fig, "Evolucion del inventario, punto de reorden (ROP) e inventario de seguridad (SS)")

    # Leyenda neutra: la ciudad ya la identifica el titulo de cada panel
    handles = [
        Line2D([], [], color=TEXT_2, linewidth=2, label="Inventario final"),
        Line2D([], [], color=TEXT_2, linestyle="none", marker="^", markersize=8, label="Pedido emitido"),
        Line2D([], [], color=SURFACE, linestyle="none", marker="o", markersize=8,
               markeredgecolor=TEXT_2, markeredgewidth=2, label="Pedido recibido"),
        Line2D([], [], color=REFERENCE, linewidth=1, linestyle="--", label="Punto de reorden"),
        Patch(color=REFERENCE, alpha=0.15, label="Inventario de seguridad"),
    ]
    fig.legend(handles=handles, loc="upper center", bbox_to_anchor=(0.5, 0.92),
               ncol=3, fontsize=8)
    return fig


def evolucion_ciudad(daily, orders, city, color):
    d = daily[daily["ciudad"] == city].sort_values("dia")
    city_orders = orders[orders["ciudad"] == city]
    last_day = d["dia"].max()

    fig = Figure(figsize=(13, 7), layout="constrained")
    fig.get_layout_engine().set(rect=(0, 0, 1, 0.84))
    inventory_ax = fig.subplots()
    demand_ax = inventory_ax.twinx()
    order_color = "#eb6834"
    demand_color = "#8a8984"

    demand_ax.bar(d["dia"], d["demanda"], width=0.72, color=demand_color, alpha=0.42,
                  label="Demanda diaria", zorder=1)
    inventory_ax.plot(d["dia"], d["inv_final"], color=color, linewidth=2.5,
                      label="Inventario final", zorder=3)

    if not city_orders.empty:
        inventory_ax.vlines(city_orders["emision"], 0, city_orders["cantidad"],
                            color=order_color, alpha=0.45, linestyles=":", zorder=2)
        inventory_ax.scatter(city_orders["emision"], city_orders["cantidad"], color=order_color,
                             edgecolors=SURFACE, linewidths=1.2, marker="D", s=75,
                             label="Pedido emitido", zorder=4)

    received = d[d["recepcion"] > 0]
    if not received.empty:
        inventory_ax.scatter(received["dia"], received["recepcion"], facecolors="none",
                             edgecolors=order_color, linewidths=2, marker="o", s=85,
                             label="Pedido recibido", zorder=5)

    rop, ss = d["rop"].iloc[0], d["ss"].iloc[0]
    inventory_ax.axhspan(0, ss, color=REFERENCE, alpha=0.12, linewidth=0, zorder=0)
    inventory_ax.axhline(rop, color=REFERENCE, linewidth=1, linestyle="--", label=f"ROP = {rop}")
    inventory_ax.axhline(ss, color=REFERENCE, linewidth=1, linestyle=":", label=f"SS = {ss}")

    inventory_ax.set_xlim(0, last_day + 1)
    inventory_ax.set_ylim(bottom=0)
    inventory_ax.set_xlabel("Día")
    inventory_ax.set_ylabel("Inventario y pedidos (unidades)")
    demand_ax.set_ylabel("Demanda diaria (unidades)")
    demand_ax.tick_params(axis="y", colors=demand_color)
    demand_ax.spines["right"].set_color(demand_color)
    demand_ax.grid(False)
    _suptitle(fig, f"Demanda, inventario y pedidos · {city}")

    handles, labels = inventory_ax.get_legend_handles_labels()
    demand_handles, demand_labels = demand_ax.get_legend_handles_labels()
    fig.legend(handles + demand_handles, labels + demand_labels, loc="upper center",
               bbox_to_anchor=(0.5, 0.92), ncol=3, fontsize=9)
    return fig


def pedidos(daily, orders, cities, colors):
    horizon = daily["dia"].max()
    fig = Figure(figsize=(9, 1.2 + 0.7 * len(cities)), layout="constrained")
    ax = fig.subplots()
    for row, city in enumerate(cities):
        for _, o in orders[orders["ciudad"] == city].iterrows():
            ax.plot([o["emision"], o["recepcion"]], [row, row], color=colors[city], linewidth=2)
            ax.plot(o["emision"], row, marker="^", markersize=8, color=colors[city],
                    markeredgecolor=SURFACE, markeredgewidth=1.5)
            ax.plot(o["recepcion"], row, marker="o", markersize=8, color=SURFACE,
                    markeredgecolor=colors[city], markeredgewidth=2)
            ax.text((o["emision"] + o["recepcion"]) / 2, row + 0.18, f"{o['cantidad']} u.",
                    ha="center", va="top", color=TEXT_2, fontsize=8)
    ax.axvline(horizon, color=REFERENCE, linewidth=1, linestyle=":")
    ax.text(horizon, -0.5, " fin del horizonte", color=TEXT_2, fontsize=8, va="center")
    ax.set_yticks(range(len(cities)), cities)
    ax.set_ylim(len(cities) - 0.4, -0.7)
    last = orders["recepcion"].max() if len(orders) else horizon
    ax.set_xlim(0, max(horizon, last) + 6)
    ax.grid(axis="x")
    ax.grid(axis="y", visible=False)
    ax.set_xlabel("Dia   (triangulo = emision, circulo = recepcion)")
    ax.set_title("Fechas de emision y recepcion de pedidos")
    return fig


def comparacion(summary, cities, colors):
    s = summary.set_index("ciudad").loc[cities]
    x = list(range(len(cities)))
    bar_colors = [colors[c] for c in cities]
    fig = Figure(figsize=(11, 3.8), layout="constrained")
    axes = fig.subplots(1, 3)

    ax = axes[0]
    ax.bar(x, s["costo_ordenar"], width=0.6, color=bar_colors, edgecolor=SURFACE, linewidth=2)
    ax.bar(x, s["costo_mantener"], width=0.6, bottom=s["costo_ordenar"], color=bar_colors,
           hatch="///", alpha=0.55, edgecolor=SURFACE, linewidth=2)
    for i, total in enumerate(s["costo_total"]):
        ax.text(i, total, f"${total:,.0f}", ha="center", va="bottom", color=TEXT, fontsize=8)
    ax.set_ylim(0, s["costo_total"].max() * 1.12)
    ax.set_title("Costo total simulado ($)")
    ax.legend(handles=[Patch(facecolor=TEXT_2, label="Ordenar"),
                       Patch(facecolor=TEXT_2, alpha=0.4, hatch="///", edgecolor=SURFACE, label="Mantener")],
              loc="upper right", fontsize=8)

    ax = axes[1]
    ax.bar(x, s["nivel_servicio"] * 100, width=0.6, color=bar_colors)
    for i, v in enumerate(s["nivel_servicio"] * 100):
        ax.text(i, v, f"{v:.1f}%", ha="center", va="bottom", color=TEXT, fontsize=8)
    ax.set_ylim(0, 108)
    ax.set_title("Nivel de servicio por unidades (%)")

    ax = axes[2]
    ax.bar(x, s["unidades_no_atendidas"], width=0.6, color=bar_colors)
    for i, (u, dias) in enumerate(zip(s["unidades_no_atendidas"], s["dias_faltante"])):
        ax.text(i, u, f"{u} u. / {dias} dias", ha="center", va="bottom", color=TEXT, fontsize=8)
    ax.set_ylim(0, max(1, s["unidades_no_atendidas"].max()) * 1.15)
    ax.set_title("Unidades no atendidas (faltantes)")

    for ax in axes:
        ax.set_xticks(x, cities, fontsize=8)
    _suptitle(fig, "Comparacion entre ciudades")
    return fig


def build_all(daily, orders, summary, cities, colors):
    """Devuelve {nombre de archivo: figura} con las cuatro graficas."""
    return {
        "demanda_diaria.png": demanda_diaria(daily, cities, colors),
        "inventario.png": inventario(daily, orders, cities, colors),
        "pedidos.png": pedidos(daily, orders, cities, colors),
        "comparacion.png": comparacion(summary, cities, colors),
    }


def main():
    if not (RESULTS / "registro_diario.csv").exists():
        print("No se encontro resultados/registro_diario.csv; corre primero ./main")
        return
    for name, fig in build_all(*load()).items():
        fig.savefig(RESULTS / name)
    print(f"Graficas guardadas en {RESULTS.name}/: demanda_diaria.png, inventario.png, pedidos.png, comparacion.png")


if __name__ == "__main__":
    main()
