#include "ui/menu.h"

#include <iomanip>
#include <iostream>

using namespace std;

double readValue(const string &prompt){
    double value;
    cout<<prompt;
    while(!(cin>>value)){
        cin.clear();
        cin.ignore(1000, '\n');
        cout<<"Valor invalido. "<<prompt;
    }
    return value;
}

MenuOption askMenuOption(){
    cout<<"=============================================================="<<endl
        <<"MENU"<<endl
        <<"  1. Correr ejemplo predeterminado"<<endl
        <<"  2. Introducir parametros manualmente"<<endl;
    int option = 0;
    while(option < 1 || option > 2) option = (int)readValue("Opcion: ");
    return (MenuOption)option;
}

InventoryParams readParams(){
    InventoryParams p;
    p.orderCost = readValue("  Costo por pedido S ($): ");
    p.holdingCost = readValue("  Costo anual de mantener una unidad H ($): ");
    p.leadTime = (int)readValue("  Tiempo de entrega L (dias): ");
    p.serviceLevel = readValue("  Nivel de servicio (ej. 0.95): ");
    p.initialInventory = (int)readValue("  Inventario inicial (unidades): ");
    return p;
}

void printUserParams(const InventoryParams &p){
    cout<<fixed<<setprecision(2)
        <<"  S = $"<<p.orderCost<<", H = $"<<p.holdingCost<<"/unidad/anio, L = "<<p.leadTime
        <<" dias, nivel de servicio = "<<p.serviceLevel<<", inventario inicial = "<<p.initialInventory<<endl;
}

ShortagePolicy readShortagePolicy(){
    cout<<"Politica de faltantes"<<endl
        <<"  1. Ventas perdidas (la demanda no atendida se pierde)"<<endl
        <<"  2. Pedidos pendientes (la demanda no atendida se surte despues)"<<endl;
    int option = 0;
    while(option != 1 && option != 2) option = (int)readValue("Opcion: ");
    return (ShortagePolicy)option;
}
