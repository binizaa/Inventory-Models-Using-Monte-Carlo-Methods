"""Interfaz de escritorio para la simulacion de inventarios EOQ con Monte Carlo.

Captura los parametros de cada ciudad, ejecuta el programa en C++ con esos
valores (opcion 2 del menu) y muestra el registro diario, los indicadores y
las graficas a partir de los CSV que deja en resultados/.

Uso:  .venv/bin/python interfaz.py
"""

import os
import re
import shutil
import subprocess
import tkinter as tk
from tkinter import font, ttk

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

import graficas

ROOT = graficas.RESULTS.parent
EXECUTABLE = ROOT / ("main.exe" if os.name == "nt" else "main")

CITIES = ["Ciudad de México", "Guadalajara", "Monterrey"]
SHORT_NAMES = {"Ciudad de México": "CDMX", "Guadalajara": "Guadalajara", "Monterrey": "Monterrey"}

# clave, etiqueta, tipo
FIELDS = [
    ("S", "Costo por pedido S ($)", float),
    ("H", "Costo anual de mantener H ($)", float),
    ("L", "Tiempo de entrega L (días)", int),
    ("nivel", "Nivel de servicio (0 a 1)", float),
    ("inv", "Inventario inicial (u)", int),
    ("pend", "Pedido pendiente (u)", int),
    ("llega", "Día de llegada del pedido", int),
]

# Mismos valores que DEFAULT_PARAMS y DEFAULT_HORIZON en src/config/config.h
DEFAULTS = {
    "Ciudad de México": {"S": 600, "H": 3.0, "L": 4, "nivel": 0.95, "inv": 500, "pend": 0, "llega": 0},
    "Guadalajara":      {"S": 500, "H": 2.5, "L": 5, "nivel": 0.95, "inv": 350, "pend": 0, "llega": 0},
    "Monterrey":        {"S": 550, "H": 2.8, "L": 3, "nivel": 0.95, "inv": 400, "pend": 0, "llega": 0},
}
DEFAULT_HORIZON = 30

PROMPTS = [
    "Opcion: ",
    "  Costo por pedido S ($): ",
    "  Costo anual de mantener una unidad H ($): ",
    "  Tiempo de entrega L (dias): ",
    "  Nivel de servicio (ej. 0.95): ",
    "  Inventario inicial (unidades): ",
    "  Pedido pendiente de recibir (unidades, 0 = ninguno): ",
    "  Dia en que llega ese pedido: ",
    "\nPASO 2  Horizonte de simulacion n (dias, mismo para todas las ciudades): ",
]

INDICATORS = [
    ("demanda_total", "Demanda total simulada", "{:,.0f}"),
    ("demanda_promedio", "Demanda promedio diaria", "{:,.2f}"),
    ("pedidos", "Pedidos emitidos", "{:,.0f}"),
    ("unidades_pedidas", "Unidades pedidas", "{:,.0f}"),
    ("inv_promedio", "Inventario promedio", "{:,.2f}"),
    ("inv_minimo", "Inventario mínimo", "{:,.0f}"),
    ("dias_faltante", "Días con faltante", "{:,.0f}"),
    ("unidades_no_atendidas", "Unidades no atendidas", "{:,.0f}"),
    ("nivel_servicio", "Nivel de servicio", "{:.2%}"),
    ("costo_ordenar", "Costo de ordenar", "${:,.2f}"),
    ("costo_mantener", "Costo de mantener", "${:,.2f}"),
    ("costo_total", "Costo total simulado", "${:,.2f}"),
]

# columna del CSV, encabezado, ancho
DAILY_COLUMNS = [
    ("dia", "Día", 44),
    ("aleatorio", "Aleatorio", 72),
    ("inv_inicial", "Inv. inicial", 78),
    ("recepcion", "Recepción", 74),
    ("demanda", "Demanda", 66),
    ("inv_disponible", "Inv. disponible", 96),
    ("faltante", "Faltante", 64),
    ("demanda_pendiente", "Dem. pendiente", 96),
    ("transito", "En tránsito", 78),
    ("prox_llegada", "Próx. llegada", 84),
    ("posicion", "Posición", 70),
    ("pedido", "Pedido", 62),
    ("inv_final", "Inv. final", 72),
]

CHART_TABS = [
    ("demanda_diaria.png", "Demanda"),
    ("inventario.png", "Inventario"),
    ("pedidos.png", "Pedidos"),
    ("comparacion.png", "Comparación"),
]


class SimulationError(Exception):
    pass


def build_input(params, horizon):
    """Texto que se le pasa a ./main por la entrada estandar (opcion 2 del menu)."""
    lines = ["2"]
    for city in CITIES:
        p = params[city]
        values = [p["S"], p["H"], p["L"], p["nivel"], p["inv"], p["pend"]]
        if p["pend"] > 0:
            values.append(p["llega"])
        lines.append(" ".join(f"{v:g}" for v in values))
    lines.append(str(horizon))
    return "\n".join(lines) + "\n"


def with_echo(output, stdin_text):
    """Escribe cada valor capturado despues de su pregunta, como en la terminal."""
    values = iter(stdin_text.split())
    pattern = re.compile("|".join(re.escape(p) for p in PROMPTS))
    return pattern.sub(lambda m: m.group(0) + next(values, "") + "\n", output)


def run_simulation(params, horizon):
    if shutil.which("make"):
        build = subprocess.run(["make"], cwd=ROOT, capture_output=True, text=True)
        if build.returncode != 0:
            raise SimulationError("No se pudo compilar el programa:\n\n" + build.stderr[-2000:])
    elif not EXECUTABLE.exists():
        raise SimulationError(f"No se encontró {EXECUTABLE.name}. Compila el programa primero.")

    stdin_text = build_input(params, horizon)
    run = subprocess.run([str(EXECUTABLE)], cwd=ROOT, input=stdin_text, capture_output=True,
                         text=True, encoding="utf-8", errors="replace", timeout=60)
    if run.returncode != 0:
        raise SimulationError("El programa terminó con error:\n\n" + (run.stderr or run.stdout)[-2000:])
    return with_echo(run.stdout, stdin_text)


class App(ttk.Frame):

    def __init__(self, master):
        super().__init__(master)
        self.master = master
        self.entries = {}
        self.figures = {}
        self.canvases = {}
        self.daily = None
        self.orders = None

        base = font.nametofont("TkDefaultFont")
        self.title_font = base.copy()
        self.title_font.configure(size=base.cget("size") + 4, weight="bold")
        self.section_font = base.copy()
        self.section_font.configure(weight="bold")
        # Colores de texto segun el tema claro u oscuro del sistema
        r, g, b = master.winfo_rgb(master.cget("background"))
        dark = (0.299 * r + 0.587 * g + 0.114 * b) / 65535 < 0.5
        self.muted = "#a3a29c" if dark else "#6b6a66"
        self.error = "#ff8a80" if dark else "#b3261e"

        style = ttk.Style()
        style.configure("Title.TLabel", font=self.title_font)
        style.configure("Section.TLabel", font=self.section_font)
        style.configure("Muted.TLabel", foreground=self.muted)

        self.grid(row=0, column=0, sticky="nsew")
        master.rowconfigure(0, weight=1)
        master.columnconfigure(0, weight=1)
        self.columnconfigure(2, weight=1)
        self.rowconfigure(0, weight=1)

        self._build_sidebar()
        ttk.Separator(self, orient="vertical").grid(row=0, column=1, sticky="ns")
        self._build_tabs()
        ttk.Separator(self, orient="horizontal").grid(row=1, column=0, columnspan=3, sticky="ew")
        self.status = ttk.Label(self, padding=(12, 4), style="Muted.TLabel")
        self.status.grid(row=2, column=0, columnspan=3, sticky="ew")

        self.reset_defaults()
        master.bind("<Return>", lambda _: self.simulate())

        if (graficas.RESULTS / "registro_diario.csv").exists():
            self.load_results()
            self.set_status("Mostrando la última simulación guardada en resultados/.")
        else:
            self.set_status("Ajusta los parámetros y presiona Simular.")

    # ----- Panel de parametros -----

    def _build_sidebar(self):
        side = ttk.Frame(self, padding=(16, 16, 16, 12))
        side.grid(row=0, column=0, sticky="ns")

        ttk.Label(side, text="Parámetros", style="Title.TLabel").grid(row=0, column=0, columnspan=4, sticky="w")
        ttk.Label(side, text="Valores de entrada por ciudad", style="Muted.TLabel").grid(
            row=1, column=0, columnspan=4, sticky="w", pady=(0, 12))

        for col, city in enumerate(CITIES, start=1):
            ttk.Label(side, text=SHORT_NAMES[city], style="Section.TLabel").grid(
                row=2, column=col, sticky="e", padx=(8, 0), pady=(0, 4))

        for row, (key, label, _) in enumerate(FIELDS, start=3):
            ttk.Label(side, text=label).grid(row=row, column=0, sticky="w", pady=3)
            for col, city in enumerate(CITIES, start=1):
                entry = ttk.Entry(side, width=8, justify="right")
                entry.grid(row=row, column=col, sticky="e", padx=(8, 0), pady=3)
                self.entries[(city, key)] = entry

        row = len(FIELDS) + 3
        ttk.Label(side, text="Si no hay pedido pendiente, deja 0 unidades.", style="Muted.TLabel").grid(
            row=row, column=0, columnspan=4, sticky="w", pady=(2, 0))

        ttk.Separator(side).grid(row=row + 1, column=0, columnspan=4, sticky="ew", pady=14)

        ttk.Label(side, text="Horizonte de simulación n (días)").grid(row=row + 2, column=0, sticky="w")
        self.horizon_entry = ttk.Entry(side, width=8, justify="right")
        self.horizon_entry.grid(row=row + 2, column=3, sticky="e", padx=(8, 0))

        buttons = ttk.Frame(side)
        buttons.grid(row=row + 3, column=0, columnspan=4, sticky="ew", pady=(18, 0))
        buttons.columnconfigure(0, weight=1)
        ttk.Button(buttons, text="Valores de ejemplo", command=self.reset_defaults).grid(row=0, column=0, sticky="w")
        ttk.Button(buttons, text="Simular", command=self.simulate, default="active").grid(row=0, column=1, sticky="e")

        side.rowconfigure(row + 4, weight=1)
        ttk.Button(side, text="Guardar gráficas como PNG", command=self.save_charts).grid(
            row=row + 5, column=0, columnspan=4, sticky="w")

    def reset_defaults(self):
        for city in CITIES:
            for key, _, _ in FIELDS:
                self._set_entry(self.entries[(city, key)], DEFAULTS[city][key])
        self._set_entry(self.horizon_entry, DEFAULT_HORIZON)

    @staticmethod
    def _set_entry(entry, value):
        entry.delete(0, "end")
        entry.insert(0, str(value))

    def read_params(self):
        """Lee y valida los campos; devuelve (params, horizonte) o None si hay un error."""
        params = {}
        for city in CITIES:
            params[city] = {}
            for key, label, kind in FIELDS:
                entry = self.entries[(city, key)]
                text = entry.get().strip().replace(",", ".")
                try:
                    number = float(text)
                    if kind is int and not number.is_integer():
                        raise ValueError
                except ValueError:
                    return self._invalid(entry, f"{label} de {city}: «{entry.get()}» no es un número válido.")
                params[city][key] = kind(number)

            p = params[city]
            if p["S"] <= 0 or p["H"] <= 0:
                return self._invalid(self.entries[(city, "S" if p["S"] <= 0 else "H")],
                                     f"Los costos de {city} deben ser mayores que 0.")
            if p["L"] < 1:
                return self._invalid(self.entries[(city, "L")], f"El tiempo de entrega de {city} debe ser al menos 1 día.")
            if not 0 < p["nivel"] < 1:
                return self._invalid(self.entries[(city, "nivel")],
                                     f"El nivel de servicio de {city} debe estar entre 0 y 1 (por ejemplo 0.95).")
            if p["inv"] < 0 or p["pend"] < 0:
                return self._invalid(self.entries[(city, "inv" if p["inv"] < 0 else "pend")],
                                     f"Las unidades de {city} no pueden ser negativas.")
            if p["pend"] > 0 and p["llega"] < 1:
                return self._invalid(self.entries[(city, "llega")],
                                     f"Indica el día (1 o más) en que llega el pedido pendiente de {city}.")

        try:
            horizon = int(self.horizon_entry.get().strip())
        except ValueError:
            horizon = 0
        if horizon < 1:
            return self._invalid(self.horizon_entry, "El horizonte debe ser un número entero de días mayor que 0.")
        return params, horizon

    def _invalid(self, entry, message):
        entry.focus_set()
        entry.select_range(0, "end")
        self.bell()
        self.set_status(message, error=True)
        return None

    # ----- Pestañas de resultados -----

    def _build_tabs(self):
        self.notebook = ttk.Notebook(self)
        self.notebook.grid(row=0, column=2, sticky="nsew", padx=12, pady=(12, 8))

        self.summary_tab = ttk.Frame(self.notebook, padding=16)
        self.notebook.add(self.summary_tab, text="Resumen")
        self._build_summary()

        self.daily_tab = ttk.Frame(self.notebook, padding=16)
        self.notebook.add(self.daily_tab, text="Registro diario")
        self._build_daily()

        self.chart_frames = {}
        for name, title in CHART_TABS:
            frame = ttk.Frame(self.notebook)
            self.notebook.add(frame, text=title)
            self.chart_frames[name] = frame

        output_tab = ttk.Frame(self.notebook, padding=(0, 8, 0, 0))
        self.notebook.add(output_tab, text="Salida del programa")
        self.output = tk.Text(output_tab, wrap="none", font="TkFixedFont", borderwidth=0,
                              highlightthickness=0, padx=12, pady=8)
        yscroll = ttk.Scrollbar(output_tab, orient="vertical", command=self.output.yview)
        xscroll = ttk.Scrollbar(output_tab, orient="horizontal", command=self.output.xview)
        self.output.configure(yscrollcommand=yscroll.set, xscrollcommand=xscroll.set, state="disabled")
        self.output.grid(row=0, column=0, sticky="nsew")
        yscroll.grid(row=0, column=1, sticky="ns")
        xscroll.grid(row=1, column=0, sticky="ew")
        output_tab.rowconfigure(0, weight=1)
        output_tab.columnconfigure(0, weight=1)

    def _build_summary(self):
        tab = self.summary_tab
        tab.columnconfigure(0, weight=1)

        ttk.Label(tab, text="Parámetros calculados", style="Section.TLabel").grid(row=0, column=0, sticky="w")
        ttk.Label(tab, text="Paso 1.2 · modelo EOQ", style="Muted.TLabel").grid(row=1, column=0, sticky="w", pady=(0, 6))
        self.params_table = self._comparison_table(tab, rows=3)
        self.params_table.grid(row=2, column=0, sticky="ew")

        ttk.Label(tab, text="Indicadores de desempeño", style="Section.TLabel").grid(
            row=3, column=0, sticky="w", pady=(20, 0))
        ttk.Label(tab, text="Paso 6 · al final del horizonte", style="Muted.TLabel").grid(
            row=4, column=0, sticky="w", pady=(0, 6))
        self.indicators_table = self._comparison_table(tab, rows=len(INDICATORS))
        self.indicators_table.grid(row=5, column=0, sticky="ew")

    def _comparison_table(self, parent, rows):
        table = ttk.Treeview(parent, columns=CITIES, height=rows, selectmode="none")
        table.heading("#0", text="", anchor="w")
        table.column("#0", width=220, stretch=False)
        for city in CITIES:
            table.heading(city, text=city, anchor="e")
            table.column(city, width=150, anchor="e")
        return table

    def _build_daily(self):
        tab = self.daily_tab
        tab.columnconfigure(0, weight=1)
        tab.rowconfigure(1, weight=1)

        bar = ttk.Frame(tab)
        bar.grid(row=0, column=0, columnspan=2, sticky="ew", pady=(0, 10))
        ttk.Label(bar, text="Ciudad").pack(side="left")
        self.city_choice = ttk.Combobox(bar, values=CITIES, state="readonly", width=18)
        self.city_choice.set(CITIES[0])
        self.city_choice.pack(side="left", padx=(8, 0))
        self.city_choice.bind("<<ComboboxSelected>>", lambda _: self.show_daily())
        ttk.Label(bar, text="Negritas: día con pedido emitido    Rojo: día con faltante",
                  style="Muted.TLabel").pack(side="right")

        keys = [key for key, _, _ in DAILY_COLUMNS]
        self.daily_table = ttk.Treeview(tab, columns=keys, show="headings", selectmode="browse")
        for key, heading, width in DAILY_COLUMNS:
            self.daily_table.heading(key, text=heading, anchor="e")
            self.daily_table.column(key, width=width, minwidth=40, anchor="e")
        bold = font.nametofont("TkDefaultFont").copy()
        bold.configure(weight="bold")
        self.daily_table.tag_configure("pedido", font=bold)
        self.daily_table.tag_configure("faltante", foreground=self.error)
        scroll = ttk.Scrollbar(tab, orient="vertical", command=self.daily_table.yview)
        self.daily_table.configure(yscrollcommand=scroll.set)
        self.daily_table.grid(row=1, column=0, sticky="nsew")
        scroll.grid(row=1, column=1, sticky="ns")

        ttk.Label(tab, text="Pedidos emitidos", style="Section.TLabel").grid(
            row=2, column=0, sticky="w", pady=(16, 6))
        order_cols = [("pedido", "Pedido", 70), ("cantidad", "Cantidad", 90),
                      ("emision", "Día de emisión", 110), ("recepcion", "Día de recepción", 120)]
        self.orders_table = ttk.Treeview(tab, columns=[c for c, _, _ in order_cols], show="headings",
                                         height=4, selectmode="none")
        for key, heading, width in order_cols:
            self.orders_table.heading(key, text=heading, anchor="e")
            self.orders_table.column(key, width=width, anchor="e", stretch=False)
        self.orders_table.grid(row=3, column=0, sticky="w")

    # ----- Acciones -----

    def simulate(self):
        values = self.read_params()
        if values is None:
            return
        params, horizon = values

        self.set_status("Simulando…")
        self.master.config(cursor="watch")
        self.update_idletasks()
        try:
            output = run_simulation(params, horizon)
        except (SimulationError, subprocess.TimeoutExpired, OSError) as error:
            self.set_status(str(error).splitlines()[0], error=True)
            self._show_output(str(error))
            self.notebook.select(len(self.notebook.tabs()) - 1)
            return
        finally:
            self.master.config(cursor="")

        self._show_output(output)
        self.load_results()
        self.set_status(f"Simulación de {horizon} días completada. Resultados guardados en resultados/.")

    def load_results(self):
        self.daily, self.orders, summary, cities, colors = graficas.load()

        first = self.daily.groupby("ciudad").first()
        self.params_table.delete(*self.params_table.get_children())
        for key, label in [("eoq", "Cantidad económica Q*"), ("ss", "Inventario de seguridad SS"),
                           ("rop", "Punto de reorden ROP")]:
            values = [f"{first.loc[c, key]:,} u." if c in first.index else "—" for c in CITIES]
            self.params_table.insert("", "end", text=label, values=values)

        by_city = summary.set_index("ciudad")
        self.indicators_table.delete(*self.indicators_table.get_children())
        for key, label, fmt in INDICATORS:
            values = [fmt.format(by_city.loc[c, key]) if c in by_city.index else "—" for c in CITIES]
            self.indicators_table.insert("", "end", text=label, values=values)

        self.show_daily()

        self.figures = graficas.build_all(self.daily, self.orders, summary, cities, colors)
        for name, frame in self.chart_frames.items():
            if name in self.canvases:
                self.canvases[name].get_tk_widget().destroy()
            canvas = FigureCanvasTkAgg(self.figures[name], master=frame)
            canvas.get_tk_widget().configure(background=graficas.SURFACE, highlightthickness=0)
            canvas.get_tk_widget().pack(fill="both", expand=True)
            canvas.draw()
            self.canvases[name] = canvas

    def show_daily(self):
        if self.daily is None:
            return
        city = self.city_choice.get()
        self.daily_table.delete(*self.daily_table.get_children())
        for _, r in self.daily[self.daily["ciudad"] == city].iterrows():
            values = []
            for key, _, _ in DAILY_COLUMNS:
                if key == "aleatorio":
                    values.append(f"{r[key]:.4f}")
                elif key == "prox_llegada" and r[key] == 0:
                    values.append("—")
                else:
                    values.append(f"{int(r[key]):,}")
            tags = []
            if r["pedido"] > 0:
                tags.append("pedido")
            if r["faltante"] > 0:
                tags.append("faltante")
            self.daily_table.insert("", "end", values=values, tags=tags)

        self.orders_table.delete(*self.orders_table.get_children())
        for _, o in self.orders[self.orders["ciudad"] == city].iterrows():
            self.orders_table.insert("", "end", values=(o["pedido"], f"{o['cantidad']:,}", o["emision"], o["recepcion"]))

    def save_charts(self):
        if not self.figures:
            self.set_status("Primero corre una simulación.", error=True)
            return
        for name, fig in self.figures.items():
            fig.savefig(graficas.RESULTS / name)
        self.set_status("Gráficas guardadas en resultados/ (" + ", ".join(self.figures) + ").")

    def _show_output(self, text):
        self.output.configure(state="normal")
        self.output.delete("1.0", "end")
        self.output.insert("1.0", text)
        self.output.configure(state="disabled")

    def set_status(self, message, error=False):
        self.status.configure(text=message, foreground=self.error if error else self.muted)


def main():
    root = tk.Tk()
    root.title("Simulación de inventarios EOQ · Monte Carlo")
    root.geometry("1360x840")
    root.minsize(1100, 680)
    App(root)
    root.mainloop()


if __name__ == "__main__":
    main()
