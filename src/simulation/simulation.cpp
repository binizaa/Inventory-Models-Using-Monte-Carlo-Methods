#include "simulation/simulation.h"
#include <iostream>
#include <random>
#include <vector>

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

// Paso 3
vector<double> generadorNumerosAleatorios(int horizon, int seed = 12345){
    vector<double> NumerosAleatorios(horizon);
    NumerosAleatorios.reserve(horizon);
    
    // establecemos una semilla para el generador de numeros aleatorios
    mt19937 generator(seed);

    // distribucion uniforme en el rango [0, 1)
    uniform_real_distribution<double> distribution(0.0, 1.0);

    for(int i = 0; i < horizon; ++i){
        NumerosAleatorios[i] = distribution(generator);
    }
    return NumerosAleatorios;
}

// Paso 4
int getDemandaSimulada(double ut, const vector<int>& demands, const vector<double>& probsAcumuladas){
    // buscamos el primer valor de la distribucion acumulada que sea mayor o igual a ut
    for (int i = 0; i < probsAcumuladas.size(); i++){
        if (probsAcumuladas[i] >= ut){
            return demands[i];
        }
    }
    // si no encontramos un valor, retornamos el ultimo valor de la distribucion
    return demands.back();
}

void correrSimulacion(Simulation &sim){ // para todo el horizonte de simulacion
    // vector<int> demands = {0, 10, 20, 30};
    // vector<double> probsAcumuladas = {0.1, 0.3, 0.7, 1.0};
    
    vector<double> aleatoriosDiarios = generadorNumerosAleatorios(sim.horizon);

    int faltanteAcumulado = 0;
    
    // ciclo de simulacion para cada dia
    for (int t = 0; t < sim.horizon; t++){
        DayRecord record;
        record.day = t + 1;

        // inventario inicial
        record.initialInventory = (t == 0) ? sim.inventory : sim.records.back().finalInventory;

        record.received = 0;
        // verificamos si hay pedidos pendientes que lleguen hoy
        for (auto it = sim.pending.begin(); it != sim.pending.end(); ){
            if (it->arrivalDay == record.day){
                record.received += it->quantity;
                sim.orders.push_back(*it); // agregamos el pedido recibido al historial de pedidos
                it = sim.pending.erase(it); // eliminamos el pedido recibido de la lista de pendientes
            } else {
                ++it;
            }
        }
        
        double Ut = aleatoriosDiarios[t];
        // obtenemos la demanda simulada para el dia t
        record.demand = getDemandaSimulada(Ut, demands, probsAcumuladas);

        // Paso 5.1 Balance de inventario

        int invDespuesDemanda = record.initialInventory + record.received - record.demand - faltanteAcumulado;
    
        if (invDespuesDemanda < 0){
            record.unmetDemand = -invDespuesDemanda;
            // guardamos el faltante acumulado para el siguiente dia
            faltanteAcumulado = record.unmetDemand;
            record.finalInventory = 0;
        } else {
            faltanteAcumulado = 0;
            record.unmetDemand = 0;
            record.finalInventory = invDespuesDemanda;
        }

        record.inventoryAfterDemand = invDespuesDemanda;

        // unidades en transito (pedidos pendientes)
        record.pendingUnits = 0;
        for (const auto& order : sim.pending){
            record.pendingUnits += order.quantity;
        }

        // Paso 5.2 Posición de inventario y decisión de reorden
        record.inventoryPosition = record.finalInventory + record.pendingUnits - faltanteAcumulado;
        record.orderPlaced = 0;

        // si la posición de inventario es menor o igual al punto de reorden y no hay en transito, emitimos un pedido
        if (record.inventoryPosition <= sim.params.reorderPoint && record.pendingUnits == 0){
            record.orderPlaced = sim.params.eoq;
            
            PendingOrder newOrder;
            newOrder.quantity = record.orderPlaced;
            newOrder.issueDay = record.day;
            newOrder.arrivalDay = record.day + sim.params.leadTime;
            sim.pending.push_back(newOrder);
            record.nextArrivalDay = newOrder.arrivalDay;

        } else {
            record.nextArrivalDay = sim.pending.empty() ? 0 : sim.pending.front().arrivalDay;
        }

        // se actualiza para el siguiente dia
        sim.inventory = record.finalInventory;

        // se registra
        sim.records.push_back(record);

        // Paso 5.3 Registro del dia
        cout << t + 1 << " \t| " 
             << Ut << " \t| " 
             << record.demand << " \t| " 
             << record.finalInventory << " \t\t| " 
             << record.inventoryPosition << " \t\t| " 
             << record.orderPlaced << " \t\t| " 
             << record.received << "\n";
    }

    // Paso 6. Indicadores de desempeño
    int demandaTotal = 0;
    int faltanteTotal = 0;
    long long sumaInventarioFinal = 0;
    int diasConFaltante = 0;
    int pedidosTotales = sim.orders.size() + sim.pending.size();

    for (const auto& rec : sim.records){
        demandaTotal += rec.demand;
        faltanteTotal += rec.unmetDemand;
        sumaInventarioFinal += rec.finalInventory;
        if (rec.unmetDemand > 0) diasConFaltante++;
    }

    double inventarioPromedio = static_cast<double>(sumaInventarioFinal) / sim.horizon;
    double serviceLevel = (demandaTotal > 0) ? (1.0 - static_cast<double>(faltanteTotal) / demandaTotal) : 1.0;
    double costoTotalOrdenes = pedidosTotales * sim.params.orderCost;
    double costoTotalMantenimiento = inventarioPromedio * sim.params.holdingCost * (sim.horizon / 365.0);
    double costoTotal = costoTotalOrdenes + costoTotalMantenimiento;

    cout << "\nIndicadores de desempeño:\n";
    cout << "Nivel de servicio por unidades: " << serviceLevel * 100 << "%\n";
    cout << "Inventario promedio diario: " << inventarioPromedio << " unidades\n";
    cout << "Número total de pedidos emitidos: " << pedidosTotales << "\n";
    cout << "Costo total de pedidos: $" << costoTotalOrdenes << "\n";
    cout << "Días con faltante: " << diasConFaltante << " días\n";
}




