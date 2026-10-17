#include "analysis/stats.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

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
