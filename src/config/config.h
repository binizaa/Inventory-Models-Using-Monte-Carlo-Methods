#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>

#include "analysis/eoq.h"
#include "simulation/simulation.h"

// ---------------------------------------------------------------------------
// Ejemplo predeterminado (opcion 1 del menu)
// S, H, L, nivel de servicio, inventario inicial
// ---------------------------------------------------------------------------

const std::string DATA_FILE = "datos.csv";

const std::map<std::string,InventoryParams> DEFAULT_PARAMS = {
    {"Ciudad de México", {600, 3.0, 4, 0.95, 500}},
    {"Guadalajara",      {500, 2.5, 5, 0.95, 350}},
    {"Monterrey",        {550, 2.8, 3, 0.95, 400}},
};

const int DEFAULT_HORIZON = 30;

// Semilla del generador de numeros aleatorios (paso 3).
// Cada ciudad usa semilla + su indice para que sus simulaciones sean independientes.
const unsigned DEFAULT_SEED = 2026;

// Politica de faltantes (paso 5.1): LOST_SALES o BACKORDERS
const ShortagePolicy DEFAULT_POLICY = LOST_SALES;

#endif
