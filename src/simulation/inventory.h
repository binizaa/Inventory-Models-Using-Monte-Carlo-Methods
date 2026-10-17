#ifndef INVENTORY_H
#define INVENTORY_H

#include "simulation/simulation.h"

// ---------------------------------------------------------------------------
// Paso 5.1 Actualizacion del inventario
//   I_t = I_{t-1} + R_t - D_t, ajustada a la politica de faltantes para que
//   el inventario fisico nunca sea negativo y los faltantes queden registrados
// ---------------------------------------------------------------------------

// Recibe los pedidos que llegan el dia t, atiende la demanda y actualiza el
// registro de ese dia (requiere la demanda simulada del paso 4)
void updateInventory(Simulation &sim, DayRecord &record);

// Aplica updateInventory a todos los dias del horizonte
void runInventory(Simulation &sim);

void printInventoryLog(const Simulation &sim);

#endif
