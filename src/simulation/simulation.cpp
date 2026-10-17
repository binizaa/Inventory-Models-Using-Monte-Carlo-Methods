#include "simulation/simulation.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

Simulation initSimulation(const string &city, const InventoryParams &params, int horizon, unsigned seed){
    Simulation sim;
    sim.city = city;
    sim.params = params;
    sim.horizon = horizon;
    sim.seed = seed;
    sim.inventory = params.initialInventory;
    if(params.initialPendingQty > 0)
        sim.pending.push_back({params.initialPendingQty, 0, params.initialPendingArrival});
    sim.records.reserve(horizon);
    return sim;
}

void printSimulationSetup(const Simulation &sim){
    int pendingUnits = 0;
    for(const PendingOrder &o : sim.pending) pendingUnits += o.quantity;

    cout<<"  Horizonte:              "<<sim.horizon<<" dias"<<endl
        <<"  Inventario inicial:     "<<sim.inventory<<endl
        <<"  Pedidos pendientes:     "<<sim.pending.size()<<" ("<<pendingUnits<<" unidades)"<<endl;
    for(const PendingOrder &o : sim.pending)
        cout<<"    - "<<o.quantity<<" unidades, llegan el dia "<<o.arrivalDay<<endl;
    cout<<"  Semilla:                "<<sim.seed<<endl
        <<"  Posicion de inventario: "<<sim.inventory + pendingUnits<<endl
        <<"  Politica:               pedir Q* = "<<sim.params.eoq
        <<" cuando posicion <= ROP = "<<sim.params.reorderPoint<<endl;
}

void printDailyRecords(const Simulation &sim){
    cout<<endl<<"PASO 5.3  Registro diario de "<<sim.city<<endl;
    cout<<right
        <<setw(4)<<"Dia"<<setw(10)<<"Aleatorio"<<setw(10)<<"Inv. ini"<<setw(10)<<"Recepcion"
        <<setw(9)<<"Demanda"<<setw(11)<<"Inv. disp."<<setw(10)<<"Faltante"<<setw(11)<<"Dem. pend."
        <<setw(11)<<"Transito"<<setw(11)<<"Prox. lleg"<<setw(10)<<"Posicion"
        <<setw(8)<<"Pedido"<<setw(10)<<"Inv. fin"<<endl;

    for(const DayRecord &r : sim.records){
        cout<<setw(4)<<r.day<<setw(10)<<fixed<<setprecision(4)<<r.random
            <<setw(10)<<r.initialInventory<<setw(10)<<r.received
            <<setw(9)<<r.demand<<setw(11)<<r.inventoryAfterDemand<<setw(10)<<r.unmetDemand<<setw(11)<<r.backorder
            <<setw(11)<<r.pendingUnits;
        if(r.nextArrivalDay > 0) cout<<setw(11)<<r.nextArrivalDay;
        else cout<<setw(11)<<"-";
        cout<<setw(10)<<r.inventoryPosition<<setw(8)<<r.orderPlaced
            <<setw(10)<<r.finalInventory<<endl;
    }
}

void printOrders(const Simulation &sim){
    cout<<endl<<"Pedidos emitidos:"<<endl;
    if(sim.orders.empty()){
        cout<<"  (ninguno)"<<endl;
        return;
    }
    cout<<right<<setw(6)<<"Pedido"<<setw(10)<<"Cantidad"<<setw(10)<<"Emision"<<setw(12)<<"Recepcion"<<endl;
    for(size_t i = 0; i < sim.orders.size(); i++){
        const PendingOrder &o = sim.orders[i];
        cout<<setw(6)<<i + 1<<setw(10)<<o.quantity<<setw(10)<<o.issueDay<<setw(12)<<o.arrivalDay;
        if(o.arrivalDay > sim.horizon) cout<<"  (despues del horizonte)";
        cout<<endl;
    }
}

// Paso 3
vector<double> generadorNumerosAleatorios(int horizon, unsigned seed){
    vector<double> NumerosAleatorios(horizon);
    NumerosAleatorios.reserve(horizon);
    
    mt19937 generator(seed);

    uniform_real_distribution<double> distribution(0.0, 1.0);

    for(int i = 0; i < horizon; ++i){
        NumerosAleatorios[i] = distribution(generator);
    }
    return NumerosAleatorios;
}

// Paso 4
int getDemandaSimulada(double ut, const vector<int>& demands, const vector<double>& probsAcumuladas){
    // buscamos el primer valor de la distribucion acumulada que sea mayor o igual a ut
    for (size_t i = 0; i < probsAcumuladas.size(); i++){
        if (probsAcumuladas[i] >= ut){
            return demands[i];
        }
    }
    return demands.back();
}

void correrSimulacion(Simulation &sim){ // para todo el horizonte de simulacion
    
    vector<double> aleatoriosDiarios = generadorNumerosAleatorios(sim.horizon, sim.seed);

    int demandaPendiente = 0;
    
    for (int t = 0; t < sim.horizon; t++){
        DayRecord record;
        record.day = t + 1;

        record.initialInventory = (t == 0) ? sim.inventory : sim.records.back().finalInventory;

        record.received = 0;
        for (auto it = sim.pending.begin(); it != sim.pending.end(); ){
            if (it->arrivalDay == record.day){
                record.received += it->quantity;
                it = sim.pending.erase(it); // eliminamos el pedido recibido de la lista de pendientes
            } else {
                ++it;
            }
        }
        
        double Ut = aleatoriosDiarios[t];
        record.random = Ut;
        record.demand = getDemandaSimulada(Ut, sim.demands, sim.probsAcumuladas);

        int disponible = record.initialInventory + record.received;

        int surtidoPendiente = min(demandaPendiente, disponible);
        disponible -= surtidoPendiente;
        demandaPendiente -= surtidoPendiente;

        int surtidoHoy = min(record.demand, disponible);
        disponible -= surtidoHoy;
        record.unmetDemand = record.demand - surtidoHoy;
        demandaPendiente += record.unmetDemand;

        record.inventoryAfterDemand = disponible;
        record.finalInventory = disponible;
        record.backorder = demandaPendiente;

        record.pendingUnits = 0;
        for (const auto& order : sim.pending){
            record.pendingUnits += order.quantity;
        }

        record.inventoryPosition = record.finalInventory + record.pendingUnits - demandaPendiente;
        record.orderPlaced = 0;

        if (record.inventoryPosition <= sim.params.reorderPoint){
            record.orderPlaced = sim.params.eoq;
            
            PendingOrder newOrder;
            newOrder.quantity = record.orderPlaced;
            newOrder.issueDay = record.day;
            newOrder.arrivalDay = record.day + sim.params.leadTime;
            sim.pending.push_back(newOrder);
            sim.orders.push_back(newOrder); // registro con fecha de emision y de recepcion
            record.pendingUnits += newOrder.quantity;
            record.nextArrivalDay = sim.pending.front().arrivalDay;

        } else {
            record.nextArrivalDay = sim.pending.empty() ? 0 : sim.pending.front().arrivalDay;
        }

        sim.inventory = record.finalInventory;

        sim.records.push_back(record);

    }

    printDailyRecords(sim);
    printOrders(sim);
}




