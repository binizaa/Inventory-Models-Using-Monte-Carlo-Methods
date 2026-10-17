#ifndef EOQ_H
#define EOQ_H

#include "analysis/stats.h"

// ---------------------------------------------------------------------------
// Paso 1.2 Parametros del modelo EOQ
// ---------------------------------------------------------------------------

struct InventoryParams
{
    // Datos que introduce el usuario
    double orderCost;        // S: costo por pedido
    double holdingCost;      // H: costo anual de mantener una unidad
    int leadTime;            // L: tiempo de entrega en dias
    double serviceLevel;     // probabilidad de no tener faltante en el ciclo
    int initialInventory;
    int initialPendingQty = 0;      // pedido pendiente al inicio (0 = ninguno)
    int initialPendingArrival = 0;  // dia en que llega ese pedido

    // Valores calculados
    double z = 0;
    double annualDemand = 0;   // D
    int eoq = 0;               // Q*
    int safetyStock = 0;       // SS
    int reorderPoint = 0;      // ROP
};

double normalCdf(double z);

// Inversa de la normal estandar por biseccion
double zFromServiceLevel(double p);

// Calcula z, D, Q*, SS y ROP a partir de los datos del usuario y la demanda
void computeEOQ(InventoryParams &p, const Stats &s);

void printParams(const InventoryParams &p);

#endif
