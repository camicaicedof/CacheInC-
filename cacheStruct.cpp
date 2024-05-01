#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>

using namespace std;

typedef pair<int, int> Pair;

enum class State
{
    Initialize,
    ReadAddress,
    CHECKING_HIT,
    REPLACING_BLOCK,
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
};

class CacheSimulator
{
private:
    State currentState;
    vector<Block> memory;
    map<string, int> isHit;
    priority_queue<Pair, vector<Pair>, greater<Pair>> leastFrecuentlyUsed;
    int totalHits;

public:
    CacheSimulator() : currentState(State::Initialize), totalHits(0) {}

    void transition(State nextState)
    {
        currentState = nextState;
    }

    void events(int memData)
    {
        switch (currentState)
        {
        case State::Initialize:
            // Initialize cache memory
            memory.clear();
            isHit.clear();
            leastFrecuentlyUsed = priority_queue<Pair, vector<Pair>, greater<Pair>>();
            totalHits = 0;
            transition(State::ReadAddress);
            break;
        case State::ReadAddress:
            readData(memData);
            break;
        case State::CHECKING_HIT:
            checkHit(memData);
            break;
        case State::REPLACING_BLOCK:
            replaceBlock(memData);
            break;
        case State::PRINTING_RESULTS:
            printResults();
            break;
        case State::FINISHED:
            // End simulation
            break;
        }
    }

    void readData(int memData)
    {
        if (memData <= 3072)
        {

            transition(State::CHECKING_HIT);
        }
        else
        {
            // Transition to PRINTING_RESULTS state
            transition(State::PRINTING_RESULTS);
        }
    }

    void checkHit(int memData)
    {
        // Check if block is present in cache
        if (isHit.find(blockAddress) != isHit.end())
        {
            // Block is present (hit)
            totalHits++;
            memory[isHit[blockAddress]].hitCount++;
            // Transition to READING state
            transition(State::READING);
        }
        else
        {
            // Block is not present (miss)
            if (memory.size() < 64)
            {
                // Cache not full, add block
                addBlock(memData);
            }
            else
            {
                // Cache full, replace block
                transition(State::REPLACING_BLOCK);
            }
        }
    }

    void addBlock(int memData)
    {
        // Add block to cache memory
        CacheEntry newEntry;
        newEntry.blockAddress = "";
        // Initialize newEntry
        memory.push_back(newEntry);
        isHit[blockAddress] = memory.size() - 1;
        // Transition to READING state
        transition(State::READING);
    }

    void replaceBlock(int memData)
    {
        // Replace block in cache memory using LRU algorithm
        // Transition to READING state
        transition(State::READING);
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