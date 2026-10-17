#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "config/config.h"
#include "data/data.h"
#include "analysis/eoq.h"
#include "ui/menu.h"
#include "simulation/simulation.h"
#include "analysis/stats.h"
#include "simulation/indicators.h"
#include "data/export.h"

using namespace std;

int main(){
    vector<deliver> delivers = readDelivers(DATA_FILE);
    if(delivers.empty()) return 1;

    map<string,vector<int>> dailyDemand = buildDailyDemand(delivers);

    cout<<"Registros leidos: "<<delivers.size()
        <<"  |  Dias observados: "<<dailyDemand.begin()->second.size()<<endl;

    bool useDefaults = askMenuOption() == DEFAULT_EXAMPLE;

    // ----- Paso 1.1 -----
    map<string,Stats> stats;
    for(auto &[city, demand] : dailyDemand){
        stats[city] = computeStats(demand);

        cout<<endl<<"=============================================================="<<endl;
        cout<<"PASO 1.1  "<<city<<endl;
        cout<<"=============================================================="<<endl;
        printFrequencyTable(buildFrequencyTable(demand));
        cout<<endl;
        printStats(stats[city]);
    }

    // ----- Paso 1.2 -----
    map<string,InventoryParams> params;
    for(auto &[city, s] : stats){
        cout<<endl<<"PASO 1.2  Parametros de inventario para "<<city<<endl;
        InventoryParams p;
        if(useDefaults && DEFAULT_PARAMS.count(city)){
            p = DEFAULT_PARAMS.at(city);
            printUserParams(p);
        }
        else p = readParams();

        computeEOQ(p, s);
        printParams(p);
        params[city] = p;
    }

    // ----- Paso 2 -----
    int horizon = DEFAULT_HORIZON;
    if(useDefaults) cout<<endl<<"PASO 2  Horizonte de simulacion n = "<<horizon<<" dias"<<endl;
    else horizon = (int)readValue("\nPASO 2  Horizonte de simulacion n (dias, mismo para todas las ciudades): ");

    vector<Simulation> simulations;
    for(auto &[city, p] : params){
        Simulation sim = initSimulation(city, p, horizon, DEFAULT_SEED + simulations.size());

        for(const FreqRow &row : buildFrequencyTable(dailyDemand[city])){
            sim.demands.push_back((int)round(row.mark));
            sim.probsAcumuladas.push_back(row.cumRelFreq);
        }

        simulations.push_back(sim);

        cout<<endl<<"PASO 2  Condiciones iniciales de "<<city<<endl;
        printSimulationSetup(simulations.back());

        // ----- Pasos 3, 4 y 5 -----
        cout<<endl<<"Ejecutando simulacion para "<<city<<"..."<<endl;
        correrSimulacion(simulations.back());
    }

    // ----- Paso 6 -----
    vector<Indicators> indicators;
    for(const Simulation &sim : simulations){
        indicators.push_back(computeIndicators(sim));

        cout<<endl<<"PASO 6  Indicadores de desempeno de "<<sim.city<<endl;
        printIndicators(indicators.back());
    }
    printComparison(simulations, indicators);
    exportResults(RESULTS_DIR, simulations, indicators);

    return 0;
}
