#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct deliver
{
    int amount;
    string date;
    string city;
    
};

int main(){
    ifstream archivo("datos.csv");

    vector<deliver> delivers;

    if(!archivo.is_open()){
        cerr<<"I's not possible to open this file"<<endl;
        return 1;
    }

    string line;
    while(getline(archivo,line)){
        stringstream ss(line);
        string celda;

        cout<<line<<endl;

        int i = 0;
        deliver newDeliver;
        while(getline(ss,celda,',')){

            //cout<<celda<<"\t";

            if(i == 1) newDeliver.amount = stoi(celda);
            else if(i == 2) newDeliver.date = celda;
            else newDeliver.city = celda;
        }
        cout<<endl;
    }

    archivo.close();
    return 0;

}