#include "simulation/inventory.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

using namespace std;

void updateInventory(Simulation &sim, DayRecord &record){
    int t = record.day;
    record.initialInventory = sim.inventory;

    record.received = 0;
    for(const PendingOrder &o : sim.pending)
        if(o.arrivalDay == t) record.received += o.quantity;
    sim.pending.erase(remove_if(sim.pending.begin(), sim.pending.end(),
                                [t](const PendingOrder &o){ return o.arrivalDay == t; }),
                      sim.pending.end());

    int available = sim.inventory + record.received;

    if(sim.policy == LOST_SALES){
        record.served = min(available, record.demand);
        record.unmetDemand = record.demand - record.served;
    }
    else{
        int requested = sim.backlog + record.demand;
        record.served = min(available, requested);
        int servedToday = max(0, record.served - sim.backlog);
        record.unmetDemand = record.demand - servedToday;
        sim.backlog = requested - record.served;
    }

    sim.inventory = available - record.served;   
    record.inventoryAfterDemand = sim.inventory;
    record.backlog = sim.backlog;

    record.pendingUnits = pendingUnits(sim);
    record.nextArrivalDay = 0;
    for(const PendingOrder &o : sim.pending)
        if(record.nextArrivalDay == 0 || o.arrivalDay < record.nextArrivalDay)
            record.nextArrivalDay = o.arrivalDay;
    record.inventoryPosition = sim.inventory + record.pendingUnits - sim.backlog;

    record.finalInventory = sim.inventory;
}

void runInventory(Simulation &sim){
    sim.inventory = sim.params.initialInventory;
    sim.backlog = 0;
    for(DayRecord &record : sim.records) updateInventory(sim, record);
}

void printInventoryLog(const Simulation &sim){
    bool backorders = sim.policy == BACKORDERS;

    cout<<right<<setw(5)<<"Dia"
        <<setw(10)<<"Inv. ini"
        <<setw(10)<<"Recibido"
        <<setw(9)<<"U_t"
        <<setw(9)<<"Demanda"
        <<setw(10)<<"Atendido"
        <<setw(10)<<"Faltante";
    if(backorders) cout<<setw(11)<<"Pendiente";
    cout<<setw(11)<<"Inv. final"
        <<setw(12)<<"En transito"
        <<setw(10)<<"Posicion"<<endl;

    int totalDemand = 0, totalUnmet = 0, shortageDays = 0;
    for(const DayRecord &r : sim.records){
        cout<<setw(5)<<r.day
            <<setw(10)<<r.initialInventory
            <<setw(10)<<r.received
            <<fixed<<setprecision(4)<<setw(9)<<r.random
            <<setw(9)<<r.demand
            <<setw(10)<<r.served
            <<setw(10)<<r.unmetDemand;
        if(backorders) cout<<setw(11)<<r.backlog;
        cout<<setw(11)<<r.finalInventory
            <<setw(12)<<r.pendingUnits
            <<setw(10)<<r.inventoryPosition<<endl;

        totalDemand += r.demand;
        totalUnmet += r.unmetDemand;
        if(r.unmetDemand > 0) shortageDays++;
    }

    cout<<"  Demanda total:          "<<totalDemand<<endl
        <<"  Dias con faltante:      "<<shortageDays<<endl
        <<"  Unidades no atendidas:  "<<totalUnmet<<endl;
    if(backorders) cout<<"  Pendiente al final:     "<<sim.backlog<<endl;
    cout<<"  Inventario final:       "<<sim.inventory<<endl;
}
