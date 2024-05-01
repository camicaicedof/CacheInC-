#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
using namespace std;

typedef pair<int, int> Pair;

struct ComparePairs
{
    bool operator()(const pair<int, int> &p1, const pair<int, int> &p2) const
    {
        if (p1.first != p2.first)
            return p1.first > p2.first; // Orden ascendente por el primer elemento del par
        else
            return p1.second > p2.second;
    }
};
// Convertir número de décimal a binario.
string decToBin(int n)
{
    int i = 0;
    string bin = "";
    while (n > 0)
    {
        bin += to_string(n % 2);
        n = n / 2;
        i++;
    }

    int x = 12 - bin.size();
    string zeros = "";
    while (x > 0)
    {
        zeros += '0';
        x--;
    }

    zeros = bin + zeros;
    string rev = string(zeros.rbegin(), zeros.rend());
    return rev;
}
// Convertir un número de binario a decimal.
int binToInt(string &num)
{
    int result = 0;
    int power = 1; // Inicializamos el valor de la potencia de 2
    // Empezamos desde el último carácter de la cadena (el bit menos significativo)
    for (int i = num.length() - 1; i >= 0; --i)
    {
        // Si el carácter es '1', agregamos 2 elevado a la potencia correspondiente
        if (num[i] == '1')
            result += power;
        // Multiplicamos la potencia por 2 para pasar al siguiente bit
        power *= 2;
    }
    return result;
}

void printMemory(vector<vector<string>> &memory)
{
    printf("id | Db | v |    Ba    | range\n");
    for (int i = 0; i < memory.size(); i++)
    {
        printf("%2d.  ", i + 1);
        for (int j = 0; j < memory[i].size(); j++)
        {

            cout << " " << memory[i][j] << "  ";
        }
        cout << endl;
    }
}
string calcularRango(int &dir, string &offset)
{
    string ans = "[";
    int offNum, limInf;
    offNum = binToInt(offset);
    limInf = dir - offNum;
    ans += to_string(limInf);
    ans += " - ";
    ans += to_string(limInf + 31);
    ans += "]";
    return ans;
}

int main()
{
    /*
       0 - dirty bit
       1 - valid bit
       2 - block adress
       3 - rango de datos
    */
    /////////////////////
    vector<vector<string>> memory; // Memoria
    map<string, int> isHit;        // recibe el BA, y mira si existe, guarda el numero de bloque
    vector<vector<string>> dataInCache (64);
    vector<int> countHits(64, 0);
    //////////////////////////
    int memData;
    int i = 0;
    bool flagPolitica = true;
    // Constantes de offset para concatenarlos al blockAddress y saber el rango de direcciones.
    const string offStart = "00000";
    string memDataS, blockAdd, offset;
    priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePairs> pq;
    while (cin >> memData && memData != '#')
    {
        if (memData <= 3072)
        {
            cout << memData << endl;
            // Se pasa a Binario para dividirlo por Bits.
            memDataS = decToBin(memData);
            blockAdd = memDataS.substr(0, 7); // blockAddress de los 7 últimos bits.
            offset = memDataS.substr(7, 5);   // Offset de 5 bits
            int num = binToInt(memDataS);
            /* cout << num << endl;
            cout << blockAdd << " " << offset << endl; */
            if (memory.size() < 64)
            {
                // Algotimo de busqueda
                // Revisamos si la clave existe
                auto it = isHit.find(blockAdd);
                i = memory.size();
                if (it != isHit.end())
                {
                    countHits[isHit[blockAdd]]++;
                    cout << "Hit! " << isHit[blockAdd] << endl;
                }
                else
                {
                    vector<string> tmp; // Crear bloque de memoria
                    string rango = calcularRango(memData, offset);
                    tmp.push_back("0");      // Dirty bit.
                    tmp.push_back("1");      // Valid bit.
                    tmp.push_back(blockAdd); // se añade al mapa el B. Address
                    tmp.push_back(rango);    // se añande el rango
                    memory.push_back(tmp);

                    // Marcar en isHit
                    // Ingresar clave block adress, valor pareja
                    isHit[blockAdd] = i;
                }

                // Hay que corregir el binario a decimal y luego añadir la primera dirección del bloque y la última (preferiblemente en decimal. O eso creo)
            }
            else if (memory.size() == 64 && flagPolitica)
            {
                printf("AQuiii\n");
                for (int j = 0; j < countHits.size(); j++)
                {
                    pair<int, int> tmp(countHits[i], i);
                    cout << countHits[j] << " " << j << endl;
                    pq.push(tmp);
                }
                flagPolitica = false;
            }
            else
            {
                // Buscar si es un hit
                auto it = isHit.find(blockAdd);
                if (it != isHit.end())
                {
                    countHits[isHit[blockAdd]]++;
                    cout << "Hit! " << isHit[blockAdd] << endl;
                }
                else
                {
                    // Encontrar el menos usado
                    bool flag = true;
                    int numBlock;
                    while (flag)
                    {
                        Pair tmp = pq.top();
                        pq.pop();
                        if (tmp.first == countHits[tmp.second])
                        {
                            numBlock = tmp.second;
                            flag = false;
                        }
                        else
                        {
                            pair<int, int> p(countHits[tmp.second], tmp.second);
                            pq.push(p);
                        }
                    }
                    // Actualizar isHit
                    isHit.erase(memory[numBlock][2]);
                    isHit[blockAdd] = numBlock;
                    // Reemplazar el miss
                    memory[numBlock][0] = "0"; // CAMBIAR CUANDO SE INGRESE WR Y RD
                    memory[numBlock][2] = blockAdd;
                    memory[numBlock][3] = calcularRango(memData, offset);
                }
            }
            cout << "Memory size: " << memory.size() << endl;
        }
        else
            printf("Numero mayor a 3072\n");
        printf("////////////////////////\n");
    }
    int totalHits = 0;
    for (int k = 0; k < memory.size(); k++)
    {
        memory[k].push_back(to_string(countHits[k]));
        totalHits += countHits[k];
    }
    printMemory(memory);
    printf("Cantidad total de Hits: %d\n", totalHits);
}