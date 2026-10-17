#include "simulation/simulation.h"

#include <iostream>

using namespace std;

Simulation initSimulation(const string &city, const InventoryParams &params, int horizon,
                          ShortagePolicy policy){
    Simulation sim;
    sim.city = city;
    sim.params = params;
    sim.horizon = horizon;
    sim.policy = policy;
    sim.inventory = params.initialInventory;
    sim.records.reserve(horizon);
    return sim;
}

string policyName(ShortagePolicy policy){
    return policy == LOST_SALES ? "ventas perdidas" : "pedidos pendientes (backorders)";
}

int pendingUnits(const Simulation &sim){
    int units = 0;
    for(const PendingOrder &o : sim.pending) units += o.quantity;
    return units;
}

void printSimulationSetup(const Simulation &sim){
    int inTransit = pendingUnits(sim);

    cout<<"  Horizonte:              "<<sim.horizon<<" dias"<<endl
        <<"  Inventario inicial:     "<<sim.inventory<<endl
        <<"  Pedidos pendientes:     "<<sim.pending.size()<<" ("<<inTransit<<" unidades)"<<endl
        <<"  Posicion de inventario: "<<sim.inventory + inTransit - sim.backlog<<endl
        <<"  Faltantes:              "<<policyName(sim.policy)<<endl
        <<"  Politica:               pedir Q* = "<<sim.params.eoq
        <<" cuando posicion <= ROP = "<<sim.params.reorderPoint<<endl
        <<"  Registro diario:        Dia | Inv. inicial | Recepcion | U_t | Demanda |"
        <<" Inv. tras demanda | Faltante | En transito | Prox. recepcion |"
        <<" Posicion | Pedido | Inv. final"<<endl;
}
