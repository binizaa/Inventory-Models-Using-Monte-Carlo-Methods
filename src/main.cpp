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
#include "simulation/montecarlo.h"
#include "simulation/inventory.h"

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

    ShortagePolicy policy = DEFAULT_POLICY;
    if(!useDefaults) policy = readShortagePolicy();

    vector<Simulation> simulations;
    for(auto &[city, p] : params){
        simulations.push_back(initSimulation(city, p, horizon, policy));

        cout<<endl<<"PASO 2  Condiciones iniciales de "<<city<<endl;
        printSimulationSetup(simulations.back());
    }

    // ----- Pasos 3 y 4 -----
    unsigned seed = DEFAULT_SEED;
    if(useDefaults) cout<<endl<<"PASO 3  Semilla del generador = "<<seed<<endl;
    else seed = (unsigned)readValue("\nPASO 3  Semilla del generador de numeros aleatorios: ");

    for(size_t i = 0; i < simulations.size(); i++){
        Simulation &sim = simulations[i];
        vector<FreqRow> table = buildFrequencyTable(dailyDemand[sim.city]);
        simulateDemand(sim, table, seed + i);

        cout<<endl<<"=============================================================="<<endl;
        cout<<"PASO 4  Demanda simulada de "<<sim.city<<"  (semilla "<<seed + i<<")"<<endl;
        cout<<"=============================================================="<<endl;
        printAssignmentTable(table);
        cout<<endl;
        printSimulatedDemand(sim, stats[sim.city]);
    }

    // ----- Paso 5.1 -----
    for(Simulation &sim : simulations){
        runInventory(sim);

        cout<<endl<<"=============================================================="<<endl;
        cout<<"PASO 5.1  Actualizacion del inventario de "<<sim.city<<"  ("<<policyName(sim.policy)<<")"<<endl;
        cout<<"=============================================================="<<endl;
        printInventoryLog(sim);
    }

    return 0;
}
