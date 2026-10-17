#include "data/export.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std;

bool exportResults(const string &dir, const vector<Simulation> &sims, const vector<Indicators> &inds){
    filesystem::create_directories(dir);

    ofstream daily(dir + "/registro_diario.csv");
    ofstream orders(dir + "/pedidos.csv");
    ofstream summary(dir + "/indicadores.csv");
    if(!daily || !orders || !summary){
        cout<<"No se pudieron escribir los archivos en "<<dir<<endl;
        return false;
    }

    daily<<"ciudad,dia,aleatorio,inv_inicial,recepcion,demanda,inv_disponible,faltante,"
           "demanda_pendiente,transito,prox_llegada,posicion,pedido,inv_final,rop,ss,eoq\n";
    orders<<"ciudad,pedido,cantidad,emision,recepcion\n";
    summary<<"ciudad,demanda_total,demanda_promedio,pedidos,unidades_pedidas,inv_promedio,"
             "inv_minimo,dias_faltante,unidades_no_atendidas,nivel_servicio,"
             "costo_ordenar,costo_mantener,costo_total\n";

    for(size_t i = 0; i < sims.size(); i++){
        const Simulation &s = sims[i];

        for(const DayRecord &r : s.records)
            daily<<s.city<<','<<r.day<<','<<fixed<<setprecision(6)<<r.random<<','
                 <<r.initialInventory<<','<<r.received<<','<<r.demand<<','
                 <<r.inventoryAfterDemand<<','<<r.unmetDemand<<','<<r.backorder<<','
                 <<r.pendingUnits<<','<<r.nextArrivalDay<<','<<r.inventoryPosition<<','
                 <<r.orderPlaced<<','<<r.finalInventory<<','
                 <<s.params.reorderPoint<<','<<s.params.safetyStock<<','<<s.params.eoq<<'\n';

        for(size_t k = 0; k < s.orders.size(); k++)
            orders<<s.city<<','<<k + 1<<','<<s.orders[k].quantity<<','
                  <<s.orders[k].issueDay<<','<<s.orders[k].arrivalDay<<'\n';

        const Indicators &ind = inds[i];
        summary<<s.city<<','<<ind.totalDemand<<','<<fixed<<setprecision(4)<<ind.avgDemand<<','
               <<ind.orders<<','<<ind.unitsOrdered<<','<<ind.avgInventory<<','
               <<ind.minInventory<<','<<ind.shortageDays<<','<<ind.unmetUnits<<','
               <<ind.serviceLevel<<','<<ind.orderingCost<<','<<ind.holdingCost<<','
               <<ind.totalCost<<'\n';
    }

    cout<<endl<<"Resultados exportados a "<<dir<<"/ (registro_diario.csv, pedidos.csv, indicadores.csv)"<<endl;
    return true;
}
