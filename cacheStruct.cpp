#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <fstream>

using namespace std;

typedef pair<int, int> Pair;

enum class State
{
    Initialize,
    ReadAddress,
    CheckHit,
    CheckMemorySize,
    ReadInstruction,
    CheckCacheForDB,
    ApplyLFU,
    UpdateRam,
    AddBlock,
    ReturnCacheData,
    UpdateBlock,
    PRINTING_RESULTS,
    FINISHED
};

struct Block
{
    bool dirtyBit;
    bool validBit;
    string blockAddress;
    int leftLimit;
    int rightLimit;
    int hitCount;
    int id;
    vector<string> data;
};
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

class CacheSimulator
{
private:
    State currentState;
    vector<Block> memory;
    map<string, int> isHit;
    priority_queue<Pair, vector<Pair>, greater<Pair>> leastFrecuentlyUsed;
    vector<string> DRAM;
    int totalHits;

public:
    CacheSimulator() : currentState(State::Initialize), totalHits(0) {}

    void transition(State nextState)
    {
        currentState = nextState;
    }

    void events(int memData, int blockId = -1)
    {
        switch (currentState)
        {
        case State::Initialize:
            cout << "Initialize" << endl;
            initialize(memData);
            break;
        case State::ReadAddress:
            cout << "read address" << endl;
            readData(memData);
            break;
        case State::CheckHit:
            cout << "CheckHit" << endl;
            checkHit(memData);
            break;
        case State::ReadInstruction:
            cout << "ReadInstruction" << endl;
            checkHit(memData);
            break;
        case State::CheckMemorySize:
            cout << "CheckMemorySize" << endl;
            checkMemorySize(memData);
            break;
        case State::ApplyLFU:
            // applyLFU(memData);
            break;
        case State::CheckCacheForDB:
            cout << "CheckCacheForDB" << endl;
            checkCacheForDB(memData);
            break;
        case State::UpdateRam:
            updateRam(memData, blockId);
            break;
        case State::UpdateBlock:
            // updateBlock(memData);
            break;
        case State::AddBlock:
            cout << "AddBlock" << endl;
            addBlock(memData);
            break;
        case State::ReturnCacheData:
            cout << "ReturnCacheData" << endl;
            // returnCacheData(memData);
            break;
        case State::PRINTING_RESULTS:
            printResults();
            break;
        case State::FINISHED:
            // End simulation
            break;
        }
    }

    void initialize(int memData)
    {
        string line;
        ifstream RAM("DRAM.txt");
        // Initialize cache memory
        memory.clear();
        isHit.clear();
        leastFrecuentlyUsed = priority_queue<Pair, vector<Pair>, greater<Pair>>();
        totalHits = 0;
        transition(State::ReadAddress);
        for (int n = 0; n < 3072; n++)
        {
            getline(RAM, line);
            DRAM.push_back(line);
        }
        events(memData);
    }

    void readData(int memData)
    {
        if (memData < 3072)
        {
            transition(State::CheckHit);
            events(memData);
        }
        else
            printf("Dir > 3072\n");
    }

    void checkHit(int memData)
    {
        string memDataS, blockAdd;
        memDataS = decToBin(memData);
        blockAdd = memDataS.substr(0, 7);
        // Hit
        if (isHit.find(blockAdd) != isHit.end())
        {
            // Block is present (hit)
            totalHits++;
            memory[isHit[blockAdd]].hitCount++;
            // Transition to READING state
            transition(State::ReadInstruction);
        }
        // Miss
        else
        {
            transition(State::CheckMemorySize);
        }
        events(memData);
    }

    void checkMemorySize(int memData)
    {
        if (memory.size() < 64)
            transition(State::CheckCacheForDB);
        else
            transition(State::ApplyLFU);
        events(memData);
    }

    void addBlock(int memData)
    {
        // Add block to cache memory
        Block newBlock;
        string memDataS, blockAdd, offset;

        memDataS = decToBin(memData);
        blockAdd = memDataS.substr(0, 7);
        offset = memDataS.substr(7, 5); // Offset de 5 bits

        newBlock.blockAddress = blockAdd;
        newBlock.dirtyBit = false;
        newBlock.validBit = true;
        newBlock.leftLimit = memData - binToInt(offset);
        newBlock.rightLimit = newBlock.leftLimit + 31;
        newBlock.id = memory.size();

        for (int i = 0; i < 32; i++)
        {
            newBlock.data.push_back(DRAM[i + newBlock.leftLimit]);
            cout << DRAM[i + newBlock.leftLimit] << endl;
        }
        // Initialize newEntry
        memory.push_back(newBlock);
        isHit[blockAdd] = newBlock.id;
        // DE MOMENTO ACABA AQUÍIIIIIIIIIIIII
        transition(State::FINISHED);
        events(memData);
    }

    void checkCacheForDB(int memData)
    {
        bool flag = true;
        int i = 0;
        while (flag && i < memory.size())
        {
            if (memory[i].dirtyBit)
                flag = false;
            else
                i++;
        }
        if (!flag)
            transition(State::UpdateRam);
        else
            transition(State::AddBlock);
        events(memData, i);
    }

    void updateRam(int memData, int id)
    {
        int pos = memory[id].leftLimit;
        for (int i = 0; i < 32; i++)
        {
            DRAM[i + pos] = memory[id].data[i];
        }
        if (memory.size() < 64)
            transition(State::AddBlock);
        else
            transition(State::AddBlock);
    }

    void printResults()
    {
        // Print cache memory and hit count
        // Transition to FINISHED state
        transition(State::FINISHED);
    }
};

int main()
{
    CacheSimulator cacheSimulator;

    int memData;
    while (cin >> memData && memData != '#')
    {
        cacheSimulator.events(memData);
    }

    return 0;
}
