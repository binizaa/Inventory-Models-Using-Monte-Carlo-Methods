# Actividad 4.4 · Simulación de inventarios EOQ con Monte Carlo

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
