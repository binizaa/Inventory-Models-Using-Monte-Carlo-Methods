#ifndef INDICATORS_H
#define INDICATORS_H

#include <vector>

#include "simulation/simulation.h"

// ---------------------------------------------------------------------------
// Paso 6. Indicadores de desempeno
// ---------------------------------------------------------------------------

struct Indicators
{
    int totalDemand = 0;          // demanda total simulada
    double avgDemand = 0;         // demanda total / n
    int orders = 0;               // pedidos emitidos
    int unitsOrdered = 0;         // unidades pedidas al proveedor
    double avgInventory = 0;      // promedio del inventario final de cada dia
    int minInventory = 0;         // menor inventario final registrado
    int shortageDays = 0;         // dias en que no se atendio toda la demanda
    int unmetUnits = 0;           // unidades no atendidas a tiempo
    double serviceLevel = 1;      // 1 - unidades no atendidas / demanda total
    double orderingCost = 0;      // pedidos * S
    double holdingCost = 0;       // inventario promedio * (H / dias de operacion) * n
    double totalCost = 0;         // costo de ordenar + costo de mantener
};

Indicators computeIndicators(const Simulation &sim);

void printIndicators(const Indicators &ind);

// Tabla con los indicadores de todas las ciudades lado a lado
void printComparison(const std::vector<Simulation> &sims, const std::vector<Indicators> &inds);

#endif
