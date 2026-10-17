# Actividad 4.4 · Simulación de inventarios EOQ con Monte Carlo

Programa en C++ que analiza la demanda histórica de un producto en tres ciudades
(Ciudad de México, Guadalajara y Monterrey) y calcula los parámetros del modelo EOQ.

Pasos implementados:

1. **Paso 1.1 · Estadística descriptiva:** tabla de frecuencias, media, desviación
   estándar, mínimo, máximo, demanda total y días con demanda cero.
2. **Paso 1.2 · Parámetros EOQ:** cantidad económica de pedido (Q\*), inventario de
   seguridad (SS) y punto de reorden (ROP).
3. **Paso 2 · Horizonte de simulación:** condiciones iniciales y estructura del registro
   diario para cada ciudad.

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
g++ -std=c++17 -Wall -Wextra -Wpedantic -Isrc -o main.exe src\main.cpp src\analysis\eoq.cpp src\analysis\stats.cpp src\data\data.cpp src\simulation\simulation.cpp src\ui\menu.cpp
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
  una unidad H, el tiempo de entrega L (días), el nivel de servicio (por ejemplo, 0.95)
  y el inventario inicial; al final pide el horizonte de simulación n.

Otros comandos:

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
├── analysis/    stats.h, stats.cpp            paso 1.1
│                eoq.h, eoq.cpp                paso 1.2
├── simulation/  simulation.h, simulation.cpp  paso 2
└── ui/          menu.h, menu.cpp              menú y captura de datos
```
