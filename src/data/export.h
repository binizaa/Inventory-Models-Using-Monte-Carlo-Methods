#ifndef EXPORT_H
#define EXPORT_H

#include <string>
#include <vector>

#include "simulation/simulation.h"
#include "simulation/indicators.h"

// ---------------------------------------------------------------------------
// Exportacion de resultados a CSV 
// ---------------------------------------------------------------------------

bool exportResults(const std::string &dir, const std::vector<Simulation> &sims,
                   const std::vector<Indicators> &inds);

#endif
