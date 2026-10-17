#include "simulation/indicators.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "analysis/stats.h"

using namespace std;

Indicators computeIndicators(const Simulation &sim){
    Indicators ind;
    if(sim.records.empty()) return ind;

    long long sumInventory = 0;
    ind.minInventory = sim.records.front().finalInventory;
    for(const DayRecord &r : sim.records){
        ind.totalDemand += r.demand;
        ind.unmetUnits += r.unmetDemand;
        sumInventory += r.finalInventory;
        ind.minInventory = min(ind.minInventory, r.finalInventory);
        if(r.unmetDemand > 0) ind.shortageDays++;
    }

    ind.orders = sim.orders.size();
    for(const PendingOrder &o : sim.orders) ind.unitsOrdered += o.quantity;

    ind.avgDemand = (double)ind.totalDemand / sim.horizon;
    ind.avgInventory = (double)sumInventory / sim.horizon;
    if(ind.totalDemand > 0) ind.serviceLevel = 1.0 - (double)ind.unmetUnits / ind.totalDemand;

    // Todos los costos sobre el horizonte de n dias; H se pasa a costo diario
    double dailyHolding = sim.params.holdingCost / DAYS_PER_YEAR;
    ind.orderingCost = ind.orders * sim.params.orderCost;
    ind.holdingCost = ind.avgInventory * dailyHolding * sim.horizon;
    ind.totalCost = ind.orderingCost + ind.holdingCost;

    return ind;
}

void printIndicators(const Indicators &ind){
    cout<<fixed<<setprecision(2)
        <<"  Demanda total simulada:     "<<ind.totalDemand<<endl
        <<"  Demanda promedio diaria:    "<<ind.avgDemand<<endl
        <<"  Pedidos emitidos:           "<<ind.orders<<" ("<<ind.unitsOrdered<<" unidades)"<<endl
        <<"  Inventario promedio:        "<<ind.avgInventory<<endl
        <<"  Inventario minimo:          "<<ind.minInventory<<endl
        <<"  Dias con faltante:          "<<ind.shortageDays<<endl
        <<"  Unidades no atendidas:      "<<ind.unmetUnits<<endl
        <<"  Nivel de servicio:          "<<ind.serviceLevel * 100<<"%"<<endl
        <<"  Costo de ordenar:           $"<<ind.orderingCost<<endl
        <<"  Costo de mantener:          $"<<ind.holdingCost<<endl
        <<"  Costo total simulado:       $"<<ind.totalCost<<endl;
}

void printComparison(const vector<Simulation> &sims, const vector<Indicators> &inds){
    cout<<endl<<"=============================================================="<<endl;
    cout<<"PASO 6  Comparacion entre ciudades"<<endl;
    cout<<"=============================================================="<<endl;

    cout<<left<<setw(26)<<"Indicador"<<right;
    for(const Simulation &s : sims) cout<<setw(18)<<s.city;
    cout<<endl;

    auto row = [&](const string &name, auto value, int precision){
        cout<<left<<setw(26)<<name<<right<<fixed<<setprecision(precision);
        for(const Indicators &ind : inds) cout<<setw(18)<<value(ind);
        cout<<endl;
    };

    row("Demanda total",          [](const Indicators &i){ return (double)i.totalDemand; }, 0);
    row("Demanda promedio diaria",[](const Indicators &i){ return i.avgDemand; }, 2);
    row("Pedidos emitidos",       [](const Indicators &i){ return (double)i.orders; }, 0);
    row("Unidades pedidas",       [](const Indicators &i){ return (double)i.unitsOrdered; }, 0);
    row("Inventario promedio",    [](const Indicators &i){ return i.avgInventory; }, 2);
    row("Inventario minimo",      [](const Indicators &i){ return (double)i.minInventory; }, 0);
    row("Dias con faltante",      [](const Indicators &i){ return (double)i.shortageDays; }, 0);
    row("Unidades no atendidas",  [](const Indicators &i){ return (double)i.unmetUnits; }, 0);
    row("Nivel de servicio (%)",  [](const Indicators &i){ return i.serviceLevel * 100; }, 2);
    row("Costo de ordenar ($)",   [](const Indicators &i){ return i.orderingCost; }, 2);
    row("Costo de mantener ($)",  [](const Indicators &i){ return i.holdingCost; }, 2);
    row("Costo total ($)",        [](const Indicators &i){ return i.totalCost; }, 2);
}
