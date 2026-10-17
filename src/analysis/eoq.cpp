#include "analysis/eoq.h"

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace std;

double normalCdf(double z){
    return 0.5 * erfc(-z / sqrt(2.0));
}

double zFromServiceLevel(double p){
    double lo = -8, hi = 8;
    for(int it = 0; it < 100; it++){
        double mid = (lo + hi) / 2;
        if(normalCdf(mid) < p) lo = mid;
        else hi = mid;
    }
    return (lo + hi) / 2;
}

void computeEOQ(InventoryParams &p, const Stats &s){
    p.z = zFromServiceLevel(p.serviceLevel);
    p.annualDemand = s.mean * DAYS_PER_YEAR;

    // Q* = sqrt(2DS / H)
    p.eoq = (int)ceil(sqrt(2 * p.annualDemand * p.orderCost / p.holdingCost));
    // SS = z * sigma_d * sqrt(L)
    p.safetyStock = (int)ceil(p.z * s.stdDev * sqrt((double)p.leadTime));
    // ROP = d * L + SS
    p.reorderPoint = (int)ceil(s.mean * p.leadTime) + p.safetyStock;
}

void printParams(const InventoryParams &p){
    cout<<fixed<<setprecision(4)
        <<"  z (nivel de servicio "<<setprecision(1)<<p.serviceLevel * 100<<"%): "
        <<setprecision(4)<<p.z<<endl
        <<setprecision(2)
        <<"  D (demanda anual):          "<<p.annualDemand<<endl
        <<"  EOQ  Q* = sqrt(2DS/H):      "<<p.eoq<<" unidades"<<endl
        <<"  SS = z * sigma_d * sqrt(L): "<<p.safetyStock<<" unidades"<<endl
        <<"  ROP = d * L + SS:           "<<p.reorderPoint<<" unidades"<<endl;
}
