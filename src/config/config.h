#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>

#include "analysis/eoq.h"


const std::string DATA_FILE = "datos.csv";

const std::map<std::string,InventoryParams> DEFAULT_PARAMS = {
    {"Ciudad de México", {600, 3.0, 4, 0.95, 500}},
    {"Guadalajara",      {500, 2.5, 5, 0.95, 350}},
    {"Monterrey",        {550, 2.8, 3, 0.95, 400}},
};

const int DEFAULT_HORIZON = 30;

#endif
