#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>

#include "analysis/eoq.h"


const std::string DATA_FILE = "datos.csv";
const std::string RESULTS_DIR = "resultados";

const std::map<std::string,InventoryParams> DEFAULT_PARAMS = {
    {"Ciudad de México", {600, 3.0, 4, 0.95, 500}},
    {"Guadalajara",      {500, 2.5, 5, 0.95, 350}},
    {"Monterrey",        {550, 2.8, 3, 0.95, 400}},
};

const int DEFAULT_HORIZON = 30;

// Semilla base; cada ciudad usa DEFAULT_SEED + su indice para simular de forma independiente
const unsigned DEFAULT_SEED = 12345;

#endif
