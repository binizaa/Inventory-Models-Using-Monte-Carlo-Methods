#ifndef SIMULATION_H
#define SIMULATION_H

#include <string>
#include <vector>

#include "analysis/eoq.h"

// ---------------------------------------------------------------------------
// Paso 2. Horizonte de simulacion y registro diario
// ---------------------------------------------------------------------------

// Que pasa con la demanda que no se puede atender por falta de inventario
enum ShortagePolicy
{
    LOST_SALES = 1,   // ventas perdidas: la demanda no atendida se pierde
    BACKORDERS = 2    // pedidos pendientes: se acumula y se surte al llegar inventario
};

struct PendingOrder
{
    int quantity;
    int issueDay;
    int arrivalDay;
};

struct DayRecord
{
    int day = 0;
    int initialInventory = 0;       // inventario inicial del dia
    int received = 0;               // recepciones del dia (R_t)
    double random = 0;              // U_t (paso 3)
    int demand = 0;                 // demanda simulada (paso 4)
    int served = 0;                 // unidades entregadas (incluye pendientes surtidos)
    int inventoryAfterDemand = 0;   // inventario fisico tras atender la demanda
    int unmetDemand = 0;            // unidades de la demanda del dia no atendidas
    int backlog = 0;                // demanda pendiente acumulada al cierre del dia
    int pendingUnits = 0;           // unidades en transito (pedidos pendientes)
    int nextArrivalDay = 0;         // proxima recepcion programada (0 = ninguna)
    int inventoryPosition = 0;      // I_t + pedidos pendientes - demanda pendiente
    int orderPlaced = 0;            // cantidad pedida ese dia (0 = no se pide)
    int finalInventory = 0;         // inventario final del dia
};

struct Simulation
{
    std::string city;
    InventoryParams params;
    int horizon;                          // n dias
    ShortagePolicy policy;
    int inventory;                        // inventario fisico actual
    int backlog = 0;                      // demanda pendiente (solo con BACKORDERS)
    std::vector<PendingOrder> pending;    // pedidos emitidos aun no recibidos
    std::vector<PendingOrder> orders;     // historial de pedidos (emision y recepcion)
    std::vector<DayRecord> records;       // registro dia t = 1..n
};

Simulation initSimulation(const std::string &city, const InventoryParams &params, int horizon,
                          ShortagePolicy policy);

std::string policyName(ShortagePolicy policy);

int pendingUnits(const Simulation &sim);

void printSimulationSetup(const Simulation &sim);

#endif
