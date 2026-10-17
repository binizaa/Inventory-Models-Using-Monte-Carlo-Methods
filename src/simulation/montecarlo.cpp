#include "simulation/montecarlo.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

using namespace std;

vector<double> generateRandoms(int n, unsigned seed){
    mt19937 generator(seed);
    uniform_real_distribution<double> uniform(0.0, 1.0);

    vector<double> randoms(n);
    for(double &u : randoms) u = uniform(generator);
    return randoms;
}

int classDemand(const FreqRow &row){
    return (int)round(row.mark);
}

int demandFromRandom(double u, const vector<FreqRow> &table){
    for(const FreqRow &row : table)
        if(row.cumRelFreq >= u) return classDemand(row);
    return classDemand(table.back());
}

void simulateDemand(Simulation &sim, const vector<FreqRow> &table, unsigned seed){
    vector<double> randoms = generateRandoms(sim.horizon, seed);

    sim.records.clear();
    for(int t = 0; t < sim.horizon; t++){
        DayRecord record;
        record.day = t + 1;
        record.random = randoms[t];
        record.demand = demandFromRandom(randoms[t], table);
        sim.records.push_back(record);
    }
}

void printAssignmentTable(const vector<FreqRow> &table){
    cout<<left<<setw(14)<<"Demanda"<<right
        <<setw(8)<<"d_i"
        <<setw(12)<<"Frec. rel"
        <<setw(10)<<"H_i"
        <<"    Intervalo de U_t"<<endl;

    double previous = 0;
    for(const FreqRow &row : table){
        if(row.absFreq == 0) continue;   // una clase vacia nunca se elige
        string interval = row.lo == row.hi ? to_string(row.lo)
                                           : to_string(row.lo) + " - " + to_string(row.hi);
        cout<<left<<setw(14)<<interval<<right<<fixed
            <<setw(8)<<classDemand(row)
            <<setw(12)<<setprecision(4)<<row.relFreq
            <<setw(10)<<setprecision(4)<<row.cumRelFreq
            <<"    "<<setprecision(4)<<previous<<" < U_t <= "<<row.cumRelFreq<<endl;
        previous = row.cumRelFreq;
    }
}

void printSimulatedDemand(const Simulation &sim, const Stats &historical){
    int total = 0;
    for(const DayRecord &r : sim.records) total += r.demand;

    double mean = sim.records.empty() ? 0 : (double)total / sim.records.size();
    cout<<fixed<<setprecision(2)
        <<"  Demanda total simulada:     "<<total<<endl
        <<"  Demanda promedio simulada:  "<<mean<<endl
        <<"  Demanda promedio historica: "<<historical.mean<<endl;
}
