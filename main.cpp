#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

using namespace std;

const int DAYS_PER_YEAR = 365;

// ---------------------------------------------------------------------------
// Lectura de datos
// ---------------------------------------------------------------------------

struct deliver
{
    int amount;
    int day;        
    string city;
};

int dayOfYear(const string &date){
    static const int monthStart[] = {0,31,59,90,120,151,181,212,243,273,304,334};
    int d, m, y;
    char sep;
    stringstream ss(date);
    ss>>d>>sep>>m>>sep>>y;
    return monthStart[m-1] + d;
}

vector<deliver> readDelivers(const string &fileName){
    vector<deliver> delivers;
    ifstream archivo(fileName);

    if(!archivo.is_open()){
        cerr<<"No es posible abrir el archivo "<<fileName<<endl;
        return delivers;
    }

    string line;
    getline(archivo,line); 

    while(getline(archivo,line)){
        if(!line.empty() && line.back() == '\r') line.pop_back();
        if(line.empty()) continue;

        stringstream ss(line);
        string celda;
        deliver newDeliver;

        getline(ss,celda,','); newDeliver.amount = stoi(celda);
        getline(ss,celda,','); newDeliver.day = dayOfYear(celda);
        getline(ss,celda,','); newDeliver.city = celda;

        delivers.push_back(newDeliver);
    }

    archivo.close();
    return delivers;
}

// ---------------------------------------------------------------------------
// Paso 1.1 Estadistica descriptiva
// ---------------------------------------------------------------------------

struct Stats
{
    int days;
    double mean;
    double stdDev;   
    int minDemand;
    int maxDemand;
    int total;
    int zeroDays;
};

struct FreqRow
{
    int lo, hi;          // intervalo de demanda [lo, hi]
    double mark;         
    int absFreq;
    double relFreq;
    int cumFreq;
    double cumRelFreq;
};

Stats computeStats(const vector<int> &demand){
    Stats s;
    s.days = demand.size();
    s.total = 0;
    s.zeroDays = 0;
    s.minDemand = *min_element(demand.begin(), demand.end());
    s.maxDemand = *max_element(demand.begin(), demand.end());

    for(int d : demand){
        s.total += d;
        if(d == 0) s.zeroDays++;
    }
    s.mean = (double)s.total / s.days;

    double sq = 0;
    for(int d : demand) sq += (d - s.mean) * (d - s.mean);
    s.stdDev = sqrt(sq / (s.days - 1));

    return s;
}

// La demanda cero se deja como clase propia; los valores positivos se agrupan
// en intervalos de igual ancho con la regla de Sturges.
vector<FreqRow> buildFrequencyTable(const vector<int> &demand){
    vector<FreqRow> table;
    int n = demand.size();

    vector<int> positive;
    for(int d : demand) if(d > 0) positive.push_back(d);
    int zeros = n - positive.size();

    if(zeros > 0) table.push_back({0, 0, 0.0, zeros, 0, 0, 0});

    if(!positive.empty()){
        int lo = *min_element(positive.begin(), positive.end());
        int hi = *max_element(positive.begin(), positive.end());
        int k = (int)ceil(1 + 3.322 * log10((double)positive.size()));
        int width = (int)ceil((double)(hi - lo + 1) / k);

        for(int a = lo; a <= hi; a += width){
            int b = a + width - 1;
            int count = 0;
            for(int d : positive) if(d >= a && d <= b) count++;
            table.push_back({a, b, (a + b) / 2.0, count, 0, 0, 0});
        }
    }

    int cum = 0;
    for(FreqRow &row : table){
        cum += row.absFreq;
        row.relFreq = (double)row.absFreq / n;
        row.cumFreq = cum;
        row.cumRelFreq = (double)cum / n;
    }

    return table;
}

void printFrequencyTable(const vector<FreqRow> &table){
    cout<<left<<setw(14)<<"Demanda"<<right
        <<setw(10)<<"Marca"
        <<setw(12)<<"Frec. abs"
        <<setw(12)<<"Frec. rel"
        <<setw(12)<<"Frec. acum"
        <<setw(16)<<"Frec. rel. acum"<<endl;

    for(const FreqRow &row : table){
        string interval = row.lo == row.hi ? to_string(row.lo)
                                           : to_string(row.lo) + " - " + to_string(row.hi);
        cout<<left<<setw(14)<<interval<<right<<fixed
            <<setw(10)<<setprecision(1)<<row.mark
            <<setw(12)<<row.absFreq
            <<setw(12)<<setprecision(4)<<row.relFreq
            <<setw(12)<<row.cumFreq
            <<setw(16)<<setprecision(4)<<row.cumRelFreq<<endl;
    }
}

void printStats(const Stats &s){
    cout<<fixed<<setprecision(2)
        <<"  Dias observados:            "<<s.days<<endl
        <<"  Media de la demanda diaria: "<<s.mean<<endl
        <<"  Desviacion estandar diaria: "<<s.stdDev<<endl
        <<"  Demanda minima:             "<<s.minDemand<<endl
        <<"  Demanda maxima:             "<<s.maxDemand<<endl
        <<"  Demanda total del periodo:  "<<s.total<<endl
        <<"  Demanda anual estimada:     "<<s.mean * DAYS_PER_YEAR<<"  (media x "<<DAYS_PER_YEAR<<")"<<endl
        <<"  Dias con demanda cero:      "<<s.zeroDays<<endl;
}

// ---------------------------------------------------------------------------
// Paso 1.2 Parametros del modelo EOQ
// ---------------------------------------------------------------------------

struct InventoryParams
{
    // Datos que introduce el usuario
    double orderCost;        // S: costo por pedido
    double holdingCost;      // H: costo anual de mantener una unidad
    int leadTime;            // L: tiempo de entrega en dias
    double serviceLevel;     // probabilidad de no tener faltante en el ciclo
    int initialInventory;

    // Valores calculados
    double z;
    double annualDemand;     // D
    int eoq;                 // Q*
    int safetyStock;         // SS
    int reorderPoint;        // ROP
};

double normalCdf(double z){
    return 0.5 * erfc(-z / sqrt(2.0));
}

// Inversa de la normal estandar por biseccion
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

// ---------------------------------------------------------------------------
// Paso 2. Horizonte de simulacion y registro diario
// ---------------------------------------------------------------------------

struct PendingOrder
{
    int quantity;
    int issueDay;
    int arrivalDay;
};

struct DayRecord
{
    int day;
    int initialInventory;       // inventario inicial del dia
    int received;               // recepciones del dia (R_t)
    double random;              // U_t (paso 3)
    int demand;                 // demanda simulada (paso 4)
    int inventoryAfterDemand;   // inventario fisico tras atender la demanda
    int unmetDemand;            // unidades no atendidas
    int pendingUnits;           // unidades en transito (pedidos pendientes)
    int nextArrivalDay;         // proxima recepcion programada (0 = ninguna)
    int inventoryPosition;      // I_t + pedidos pendientes - demanda pendiente
    int orderPlaced;            // cantidad pedida ese dia (0 = no se pide)
    int finalInventory;         // inventario final del dia
};

struct Simulation
{
    string city;
    InventoryParams params;
    int horizon;                     // n dias
    int inventory;                   // inventario fisico actual
    vector<PendingOrder> pending;    // pedidos emitidos aun no recibidos
    vector<PendingOrder> orders;     // historial de pedidos (emision y recepcion)
    vector<DayRecord> records;       // registro dia t = 1..n
};

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
        <<"  Registro diario:        Dia | Inv. inicial | Recepcion | U_t | Demanda |"
        <<" Inv. tras demanda | Faltante | En transito | Prox. recepcion |"
        <<" Posicion | Pedido | Inv. final"<<endl;
}

// ---------------------------------------------------------------------------

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

int main(){
    vector<deliver> delivers = readDelivers("datos.csv");
    if(delivers.empty()) return 1;

    // Periodo observado comun a las tres ubicaciones
    int firstDay = delivers[0].day, lastDay = delivers[0].day;
    for(const deliver &d : delivers){
        firstDay = min(firstDay, d.day);
        lastDay = max(lastDay, d.day);
    }
    int numDays = lastDay - firstDay + 1;

    // Demanda diaria = suma de los pedidos de cada dia; dias sin pedidos = 0
    map<string,vector<int>> dailyDemand;
    for(const deliver &d : delivers){
        if(!dailyDemand.count(d.city)) dailyDemand[d.city] = vector<int>(numDays, 0);
        dailyDemand[d.city][d.day - firstDay] += d.amount;
    }

    cout<<"Registros leidos: "<<delivers.size()<<"  |  Dias observados: "<<numDays<<endl;

    // ----- Paso 1 -----
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

    map<string,InventoryParams> params;
    for(auto &[city, s] : stats){
        cout<<endl<<"PASO 1.2  Parametros de inventario para "<<city<<endl;
        InventoryParams p;
        p.orderCost = readValue("  Costo por pedido S ($): ");
        p.holdingCost = readValue("  Costo anual de mantener una unidad H ($): ");
        p.leadTime = (int)readValue("  Tiempo de entrega L (dias): ");
        p.serviceLevel = readValue("  Nivel de servicio (ej. 0.95): ");
        p.initialInventory = (int)readValue("  Inventario inicial (unidades): ");

        computeEOQ(p, s);
        printParams(p);
        params[city] = p;
    }

    // ----- Paso 2 -----
    int horizon = (int)readValue("\nPASO 2  Horizonte de simulacion n (dias, mismo para todas las ciudades): ");

    vector<Simulation> simulations;
    for(auto &[city, p] : params){
        simulations.push_back(initSimulation(city, p, horizon));

        cout<<endl<<"PASO 2  Condiciones iniciales de "<<city<<endl;
        printSimulationSetup(simulations.back());
    }

    return 0;
}
