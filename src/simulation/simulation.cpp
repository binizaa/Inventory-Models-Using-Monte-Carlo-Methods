#include "simulation/simulation.h"

#include <iostream>

using namespace std;

Simulation initSimulation(const string &city, const InventoryParams &params, int horizon){
    Simulation sim;
    sim.city = city;
    sim.params = params;
    sim.horizon = horizon;
    sim.inventory = params.initialInventory;
    sim.records.reserve(horizon);
    return sim;
}

void printSimulationSetup(const Simulation &sim){
    int pendingUnits = 0;
    for(const PendingOrder &o : sim.pending) pendingUnits += o.quantity;

    cout<<"  Horizonte:              "<<sim.horizon<<" dias"<<endl
        <<"  Inventario inicial:     "<<sim.inventory<<endl
        <<"  Pedidos pendientes:     "<<sim.pending.size()<<" ("<<pendingUnits<<" unidades)"<<endl
        <<"  Posicion de inventario: "<<sim.inventory + pendingUnits<<endl
        <<"  Politica:               pedir Q* = "<<sim.params.eoq
        <<" cuando posicion <= ROP = "<<sim.params.reorderPoint<<endl
        <<"  Registro diario:        Dia | Inv. inicial | Recepcion | Demanda |"
        <<" Inv. tras demanda | Faltante | En transito | Prox. recepcion |"
        <<" Posicion | Pedido | Inv. final"<<endl;
}
