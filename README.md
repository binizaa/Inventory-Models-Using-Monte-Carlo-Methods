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

- Un compilador de C++17 (`g++` o `clang++`)
- `make`

## Cómo correrlo

Desde la carpeta de la actividad:

```bash
cd "Actividad 4.4"
make run
```

`make run` compila el programa y lo ejecuta. Debe correrse desde esta carpeta para que
encuentre `datos.csv`.

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
make         # solo compila (genera el ejecutable ./main)
./main       # ejecuta el programa ya compilado
make clean   # borra el ejecutable y los archivos .o
```

Para compilar sin `make`:

```bash
g++ -std=c++17 -Isrc -o main src/*.cpp src/*/*.cpp
./main
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
