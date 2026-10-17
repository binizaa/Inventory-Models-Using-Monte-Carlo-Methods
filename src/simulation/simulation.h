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
    int day = 0;
    double random = 0;              // U_t
    int initialInventory = 0;       // inventario inicial del dia
    int received = 0;               // recepciones del dia (R_t)
    int demand = 0;                 // demanda simulada
    int inventoryAfterDemand = 0;   // inventario fisico tras atender la demanda
    int unmetDemand = 0;            // unidades de la demanda del dia que no se atendieron
    int backorder = 0;              // demanda pendiente acumulada al cierre del dia
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
    unsigned seed;                        // semilla del generador de esta ciudad
    int inventory;                        // inventario fisico actual
    std::vector<PendingOrder> pending;    // pedidos emitidos aun no recibidos
    std::vector<PendingOrder> orders;     // pedidos emitidos en la simulacion (emision y recepcion)
    std::vector<DayRecord> records;       // registro dia t = 1..n
    std::vector<int> demands;
    std::vector<double> probsAcumuladas;
};

Simulation initSimulation(const std::string &city, const InventoryParams &params, int horizon, unsigned seed);

void printSimulationSetup(const Simulation &sim);

void printDailyRecords(const Simulation &sim);

void printOrders(const Simulation &sim);

void correrSimulacion(Simulation &sim);

#endif
