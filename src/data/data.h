#ifndef DATA_H
#define DATA_H

#include <map>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Lectura de datos
// ---------------------------------------------------------------------------

struct deliver
{
    int amount;
    int day;      
    std::string city;
};

// Convierte "dd/mm/aaaa" a dia del anio (anio no bisiesto)
int dayOfYear(const std::string &date);

std::vector<deliver> readDelivers(const std::string &fileName);

std::map<std::string,std::vector<int>> buildDailyDemand(const std::vector<deliver> &delivers);

#endif
