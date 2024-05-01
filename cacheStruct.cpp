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
    ChangeCacheData,
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
    bool iniLFU;

public:
    CacheSimulator() : currentState(State::Initialize), totalHits(0), iniLFU(true) {}

    void transition(State nextState)
    {
        currentState = nextState;
    }

    void events(int memData, string instruction, string dataChange, int blockId = -1)
    {
        switch (currentState)
        {
        case State::Initialize:
            cout << "Initialize" << endl;
            initialize(memData, instruction, dataChange);
            break;
        case State::ReadAddress:
            cout << "read address" << endl;
            readData(memData, instruction, dataChange);
            break;
        case State::CheckHit:
            cout << "CheckHit" << endl;
            checkHit(memData, instruction, dataChange);
            break;
        case State::ReadInstruction:
            cout << "ReadInstruction" << endl;
            readInstruction(memData, instruction, dataChange, blockId);
            break;
        case State::CheckMemorySize:
            cout << "CheckMemorySize" << endl;
            checkMemorySize(memData, instruction, dataChange);
            break;
        case State::ApplyLFU:
            cout << "ApplyLFU" << endl;
            applyLFU(memData, instruction, dataChange, blockId);
            break;
        case State::CheckCacheForDB:
            cout << "CheckCacheForDB" << endl;
            checkCacheForDB(memData, instruction, dataChange);
            break;
        case State::UpdateRam:
            cout << "UpdateRam" << endl;
            updateRam(memData, instruction, dataChange, blockId);
            break;
        case State::UpdateBlock:
            cout << "UpdateBlock" << endl;
            updateBlock(memData, instruction, dataChange, blockId);
            break;
        case State::AddBlock:
            cout << "AddBlock" << endl;
            addBlock(memData, instruction, dataChange);
            break;
        case State::ReturnCacheData:
            cout << "ReturnCacheData" << endl;
            returnCacheData(memData, instruction, dataChange, blockId);
            break;
        case State::ChangeCacheData:
            cout << "ChangeCacheData" << endl;
            changeCacheData(memData, instruction, dataChange, blockId);
            break;
        case State::PRINTING_RESULTS:
            printResults();
            break;
        case State::FINISHED:
            // End simulation
            break;
        }
    }

    void initialize(int memData, string instruction, string dataChange)
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
        events(memData, instruction, dataChange);
    }

    void readData(int memData, string instruction, string dataChange)
    {
        if (memData < 3072)
        {
            transition(State::CheckHit);
            events(memData, instruction, dataChange);
        }
        else
            printf("Dir > 3072\n");
    }

    void checkHit(int memData, string instruction, string dataChange)
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
        events(memData, instruction, dataChange);
    }

    void checkMemorySize(int memData, string instruction, string dataChange)
    {
        if (memory.size() < 64)
            transition(State::CheckCacheForDB);
        else
            transition(State::ApplyLFU);
        events(memData, instruction, dataChange);
    }

    void addBlock(int memData, string instruction, string dataChange)
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
        }
        // Initialize newEntry
        memory.push_back(newBlock);
        isHit[blockAdd] = newBlock.id;
        transition(State::ReadInstruction);
        events(memData, instruction, dataChange, newBlock.id);
    }

    void checkCacheForDB(int memData, string instruction, string dataChange)
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
        events(memData, instruction, dataChange, i);
    }
    
    void readInstruction(int memData, string instruction, string dataChange, int id){
        if (instruction == "Rd")
            transition(State::ReturnCacheData);
        else if (instruction == "Wr")
            transition(State::ChangeCacheData);
        events(memData, instruction, dataChange, id);
    }

    void returnCacheData(int memData, string instruction, string dataChange, int id){
        //En caso de que se pete, poner el offset jaja
        cout << memory[id].data[memData-memory[id].leftLimit]<<endl;
        transition(State::ReadAddress);
    }

    void changeCacheData(int memData, string instruction, string dataChange, int id){
        memory[id].data[memData-memory[id].leftLimit] = dataChange;
        memory[id].dirtyBit = true;
        transition(State::ReadAddress);
    }

    void updateBlock(int memData, string instruction, string dataChange, int id)
    {
        string memDataS, blockAdd, offset;

        memDataS = decToBin(memData);
        blockAdd = memDataS.substr(0, 7);
        offset = memDataS.substr(7, 5); // Offset de 5 bits
        // Update keys for Hits
        isHit.erase(memory[id].blockAddress);
        isHit[blockAdd] = id;
        // Update the block
        memory[id].blockAddress = blockAdd;
        memory[id].validBit = true;
        memory[id].dirtyBit = false;
        memory[id].leftLimit = memData - binToInt(offset);
        memory[id].rightLimit = memory[id].leftLimit + 31;

        for (int i = 0; i < 32; i++)
            memory[id].data.push_back(DRAM[i + memory[id].leftLimit]);
        transition(State::ReadInstruction);
        events(memData, instruction, dataChange, id);
    }
    
    void applyLFU(int memData, string instruction, string dataChange, int id)
    {
        if (iniLFU)
        {
            for (int j = 0; j < memory.size(); j++)
            {
                pair<int, int> tmp(memory[j].hitCount, j);
                leastFrecuentlyUsed.push(tmp);
            }
            iniLFU = false;
        }
        bool flag = true;
        int numBlock;
        while (flag)
        {
            Pair tmp = leastFrecuentlyUsed.top();
            leastFrecuentlyUsed.pop();
            if (tmp.first == memory[tmp.second].hitCount)
            {
                numBlock = tmp.second;
                flag = false;
            }
            else
            {
                pair<int, int> p(memory[tmp.second].hitCount, tmp.second);
                leastFrecuentlyUsed.push(p);
            }
        }
    }

    void updateRam(int memData, string instruction, string dataChange, int id)
    {
        int pos = memory[id].leftLimit;
        for (int i = 0; i < 32; i++)
        {
            DRAM[i + pos] = memory[id].data[i];
        }
        if (memory.size() < 64)
            transition(State::AddBlock);
        else
            transition(State::UpdateBlock);
        events(memData, instruction, dataChange, id);
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
    int address;
    string RoW, change;
    while (cin>>RoW)
    {   cin >> address;
        if (RoW == "Wr"){
            cin >> change;
        }
        
        cacheSimulator.events(address, RoW, change);
    
    }
    return 0;
}
