#ifndef STATS_H
#define STATS_H

#include <vector>

const int DAYS_PER_YEAR = 365;

// ---------------------------------------------------------------------------
// Paso 1.1 Estadistica descriptiva
// ---------------------------------------------------------------------------

struct Stats
{
    int days;
    double mean;
    double stdDev;
    int minDemand;
    int maxDemand;
    int total;
    int zeroDays;
};

struct FreqRow
{
    int lo, hi;          // intervalo de demanda [lo, hi]
    double mark;
    int absFreq;
    double relFreq;
    int cumFreq;
    double cumRelFreq;
};

Stats computeStats(const std::vector<int> &demand);

// La demanda cero se deja como clase propia; los valores positivos se agrupan
// en intervalos de igual ancho con la regla de Sturges.
std::vector<FreqRow> buildFrequencyTable(const std::vector<int> &demand);

void printFrequencyTable(const std::vector<FreqRow> &table);
void printStats(const Stats &s);

#endif
