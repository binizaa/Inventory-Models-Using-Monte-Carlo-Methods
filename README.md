# Actividad 4.4 · Simulación de inventarios EOQ con Monte Carlo

Este repositorio implementa una simulación de inventarios para varias ciudades, combinando el modelo EOQ (cantidad económica de pedido) con generación de demanda mediante Monte Carlo. Permite configurar los parámetros, ejecutar simulaciones para un horizonte definido, consultar indicadores y visualizar los resultados.

El proyecto utiliza **C++17** para los cálculos de análisis y simulación, y **Python 3** para la interfaz gráfica y las visualizaciones. La interfaz está construida con Tkinter; las gráficas y la lectura de resultados utilizan Matplotlib y pandas. El programa C++ se compila con `g++` o `make`.

## Cómo correr la interfaz

Desde la raíz del repositorio.

### macOS

La primera vez, instala las dependencias:

```bash
brew install python-tk@3.14          # Tkinter, con la versión de tu Python
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt
```

Después, para abrir la interfaz:

```bash
.venv/bin/python interfaz.py
```

La interfaz compila el programa en C++ con `make` antes de cada simulación.

### Windows

La primera vez, instala las dependencias:

```powershell
python -m venv .venv
.venv\Scripts\pip install -r requirements.txt
```

Compila el programa (o usa **F5** en VS Code con MSYS2 UCRT64):

```powershell
g++ -std=c++17 -Isrc -o main.exe src\main.cpp src\analysis\eoq.cpp src\analysis\stats.cpp src\data\data.cpp src\data\export.cpp src\simulation\simulation.cpp src\simulation\indicators.cpp src\ui\menu.cpp
```

Después, para abrir la interfaz:

```powershell
.venv\Scripts\python interfaz.py
```

### Uso

Ajusta los parámetros de cada ciudad (o usa **Valores de ejemplo**), indica el horizonte
de simulación y presiona **Simular**. Los resultados se muestran en las pestañas y se
guardan en `resultados/`.

## Archivos del proyecto

### Archivos principales

- `datos.csv`: datos históricos de demanda con cantidad, fecha y ubicación; sirven como entrada para el análisis y la simulación.
- `graficas.py`: carga los resultados CSV y genera las gráficas de demanda, inventario, pedidos y comparación entre ciudades.
- `interfaz.py`: interfaz de escritorio para configurar y ejecutar simulaciones, y consultar sus resultados.
- `requirements.txt`: dependencias de Python, actualmente Matplotlib y pandas.
- `Makefile`: reglas para compilar los archivos C++ con `g++`, ejecutar el programa y limpiar los objetos generados.
- `.gitignore`: excluye del control de versiones el entorno virtual, ejecutables, objetos y caché de Python.
- `5.3.- Actividad de simulación MonteCarlo.pdf`: documento de referencia de la actividad.
- `5.3.- demanda_simulada_tres_ubicaciones_mexico.xlsx`: hoja de cálculo de referencia con datos de demanda para tres ubicaciones.

### Código C++ (`src/`)

- `main.cpp`: punto de entrada; coordina la lectura de datos, el análisis, la simulación y la exportación de resultados.
- `config/config.h`: constantes de configuración, archivo de datos, carpeta de salida y parámetros predeterminados por ciudad.
- `analysis/eoq.cpp` y `analysis/eoq.h`: calculan la cantidad económica de pedido (EOQ), el punto de reorden y los parámetros asociados.
- `analysis/stats.cpp` y `analysis/stats.h`: calculan estadísticas y tablas de frecuencia de la demanda.
- `data/data.cpp` y `data/data.h`: leen los pedidos del CSV y agrupan la demanda por día y ubicación.
- `data/export.cpp` y `data/export.h`: exportan a CSV los registros, pedidos e indicadores de las simulaciones.
- `simulation/indicators.cpp` y `simulation/indicators.h`: calculan y presentan indicadores de inventario, servicio y costos.
- `simulation/simulation.cpp` y `simulation/simulation.h`: inicializan y ejecutan las simulaciones, generan demanda y muestran registros diarios.
- `ui/menu.cpp` y `ui/menu.h`: solicitan parámetros por consola y muestran opciones e información de la simulación.

### Resultados de ejemplo (`resultados/`)

- `comparacion.png`: gráfica comparativa de indicadores entre ciudades.
- `demanda_diaria.png`: visualización de la demanda diaria por ciudad.
- `inventario.png`: evolución del inventario durante la simulación.
- `pedidos.png`: visualización de los pedidos realizados y sus recepciones.
- `indicadores.csv`: resumen de demanda, inventario, nivel de servicio y costos por ciudad.
- `pedidos.csv`: cantidades y fechas de emisión y recepción de cada pedido.
- `registro_diario.csv`: detalle diario de demanda, inventario, pedidos y parámetros de reabastecimiento.
