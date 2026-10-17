#ifndef MONTECARLO_H
#define MONTECARLO_H

#include <vector>

#include "analysis/stats.h"
#include "simulation/simulation.h"

// ---------------------------------------------------------------------------
// Paso 3. Numeros aleatorios U_t ~ U(0,1)
// Paso 4. Transformacion de U_t en demanda simulada
// ---------------------------------------------------------------------------

// Genera n numeros uniformes en [0,1) con semilla fija (resultados reproducibles)
std::vector<double> generateRandoms(int n, unsigned seed);

// Valor de demanda d_i de cada clase: marca de clase redondeada
int classDemand(const FreqRow &row);

// Primer d_i cuya frecuencia relativa acumulada cumple H_i >= U_t
int demandFromRandom(double u, const std::vector<FreqRow> &table);

// Llena U_t y D_t en el registro diario t = 1..n
void simulateDemand(Simulation &sim, const std::vector<FreqRow> &table, unsigned seed);

// Tabla de asignacion: intervalo de U_t -> demanda d_i
void printAssignmentTable(const std::vector<FreqRow> &table);

// Resumen de la demanda simulada comparada con la historica
void printSimulatedDemand(const Simulation &sim, const Stats &historical);

#endif
