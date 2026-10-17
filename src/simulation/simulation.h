#ifndef SIMULATION_H
#define SIMULATION_H

#include <string>
#include <vector>

#include "analysis/eoq.h"

// ---------------------------------------------------------------------------
// Paso 2. Horizonte de simulacion y registro diario
// ---------------------------------------------------------------------------

struct PendingOrder
{
    int quantity;
    int issueDay;
    int arrivalDay;
};

struct DayRecord
{
    int day;
    int initialInventory;       // inventario inicial del dia
    int received;               // recepciones del dia (R_t)
    double random;              // U_t (paso 3)
    int demand;                 // demanda simulada (paso 4)
    int inventoryAfterDemand;   // inventario fisico tras atender la demanda
    int unmetDemand;            // unidades no atendidas
    int pendingUnits;           // unidades en transito (pedidos pendientes)
    int nextArrivalDay;         // proxima recepcion programada (0 = ninguna)
    int inventoryPosition;      // I_t + pedidos pendientes - demanda pendiente
    int orderPlaced;            // cantidad pedida ese dia (0 = no se pide)
    int finalInventory;         // inventario final del dia
};

struct Simulation
{
    std::string city;
    InventoryParams params;
    int horizon;                          // n dias
    int inventory;                        // inventario fisico actual
    std::vector<PendingOrder> pending;    // pedidos emitidos aun no recibidos
    std::vector<PendingOrder> orders;     // historial de pedidos (emision y recepcion)
    std::vector<DayRecord> records;       // registro dia t = 1..n
};

Simulation initSimulation(const std::string &city, const InventoryParams &params, int horizon);

void printSimulationSetup(const Simulation &sim);

#endif
