#include "data/data.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

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

map<string,vector<int>> buildDailyDemand(const vector<deliver> &delivers){
    map<string,vector<int>> dailyDemand;
    if(delivers.empty()) return dailyDemand;

    // Periodo observado comun a las tres ubicaciones
    int firstDay = delivers[0].day, lastDay = delivers[0].day;
    for(const deliver &d : delivers){
        firstDay = min(firstDay, d.day);
        lastDay = max(lastDay, d.day);
    }
    int numDays = lastDay - firstDay + 1;

    for(const deliver &d : delivers){
        if(!dailyDemand.count(d.city)) dailyDemand[d.city] = vector<int>(numDays, 0);
        dailyDemand[d.city][d.day - firstDay] += d.amount;
    }

    return dailyDemand;
}
