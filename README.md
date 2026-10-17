# Actividad 4.4 · Simulación de inventarios EOQ con Monte Carlo

Programa en C++ que analiza la demanda histórica de un producto en tres ciudades
(Ciudad de México, Guadalajara y Monterrey), calcula los parámetros del modelo EOQ y
simula el inventario de cada ciudad con el método de Monte Carlo.

Pasos implementados:

1. **Paso 1.1 · Estadística descriptiva:** tabla de frecuencias, media, desviación
   estándar, mínimo, máximo, demanda total y días con demanda cero.
2. **Paso 1.2 · Parámetros EOQ:** cantidad económica de pedido (Q\*), inventario de
   seguridad (SS) y punto de reorden (ROP).
3. **Paso 2 · Horizonte de simulación:** condiciones iniciales (inventario y pedido
   pendiente) y registro diario para cada ciudad.
4. **Paso 3 · Números aleatorios:** U<sub>t</sub> ~ U(0,1) por día con semilla fija; cada
   ciudad usa su propia semilla.
5. **Paso 4 · Demanda simulada:** primer d<sub>i</sub> de la tabla de frecuencias con
   H<sub>i</sub> ≥ U<sub>t</sub> (d<sub>i</sub> es la marca de clase del intervalo).
6. **Paso 5 · Inventario y resurtido:** balance diario con faltantes que se surten después
   (backorder), pedido de Q\* cuando la posición de inventario es ≤ ROP, y registro diario
   y de pedidos.
7. **Paso 6 · Indicadores y gráficas:** indicadores por ciudad, tabla comparativa,
   exportación a CSV y gráficas con `graficas.py`.

## Requisitos

- Un compilador de C++17 (`g++`); en Windows se recomienda MSYS2 UCRT64.
- `make` solo es necesario para usar los comandos del `Makefile`.

## Cómo correrlo

Abre **la carpeta raíz del repositorio** en VS Code. En Windows, con MSYS2 UCRT64
instalado, puedes compilar y depurar el proyecto con **F5**. La tarea de VS Code compila
todos los archivos necesarios, y la configuración de depuración usa la raíz como
directorio de trabajo para encontrar `datos.csv`.

Para compilar y ejecutar manualmente desde PowerShell, en la raíz del repositorio:

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -Isrc -o main.exe src\main.cpp src\analysis\eoq.cpp src\analysis\stats.cpp src\data\data.cpp src\data\export.cpp src\simulation\simulation.cpp src\simulation\indicators.cpp src\ui\menu.cpp
.\main.exe
```

Si ejecutas `g++` desde una terminal normal de Windows, añade
`C:\msys64\ucrt64\bin` y `C:\msys64\usr\bin` al `PATH` y abre una terminal nueva. Esto
permite que el compilador encuentre sus herramientas y que el programa encuentre sus DLL
al ejecutarse.

Si tienes `make` instalado, los comandos del `Makefile` también deben ejecutarse desde
la raíz del repositorio:

```bash
make run
```

Al iniciar aparece un menú:

```
MENU
  1. Correr ejemplo predeterminado
  2. Introducir parametros manualmente
```

- **Opción 1:** usa los parámetros de ejemplo definidos en `src/config/config.h` y un
  horizonte de 30 días.
- **Opción 2:** pide para cada ciudad el costo por pedido S, el costo anual de mantener
  una unidad H, el tiempo de entrega L (días), el nivel de servicio (por ejemplo, 0.95),
  el inventario inicial y, si hay, un pedido pendiente de recibir (unidades y día de
  llegada); al final pide el horizonte de simulación n.

Al terminar, el programa guarda los resultados en `resultados/`
(`registro_diario.csv`, `pedidos.csv`, `indicadores.csv`).

## Gráficas (paso 6)

Requieren Python 3 con `matplotlib` y `pandas`. Después de correr el programa:

```bash
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt     # Windows: .venv\Scripts\pip install -r requirements.txt
.venv/bin/python graficas.py                  # Windows: .venv\Scripts\python graficas.py
```

Se generan en `resultados/`:

- `demanda_diaria.png`: demanda diaria simulada por ubicación.
- `inventario.png`: evolución del inventario con punto de reorden, inventario de seguridad
  y días de emisión y recepción de pedidos.
- `pedidos.png`: fechas de emisión y recepción de cada pedido.
- `comparacion.png`: costos, nivel de servicio y faltantes entre las tres ciudades.

## Otros comandos


```bash
make         # solo compila (genera el ejecutable main)
./main       # ejecuta el programa ya compilado (Windows: .\main.exe)
make clean   # borra el ejecutable y los archivos .o
```

## Datos

`datos.csv` contiene 1,200 pedidos del 01/01/2026 al 10/10/2026 con las columnas
`Cantidad demandada, Fecha del pedido, Ubicación`. La demanda diaria de cada ciudad es
la suma de sus pedidos en cada fecha; los días sin pedidos cuentan como demanda 0.

## Estructura

```
src/
├── main.cpp
├── config/      config.h                      parámetros del ejemplo predeterminado
├── data/        data.h, data.cpp              lectura del CSV y demanda diaria
│                export.h, export.cpp          exportación de resultados a CSV
├── analysis/    stats.h, stats.cpp            paso 1.1
│                eoq.h, eoq.cpp                paso 1.2
├── simulation/  simulation.h, simulation.cpp  pasos 2 a 5
│                indicators.h, indicators.cpp  paso 6
└── ui/          menu.h, menu.cpp              menú y captura de datos
graficas.py                                    gráficas del paso 6
```
