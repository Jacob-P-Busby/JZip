#include "CharHeap.h"


CharHeap::CharHeap(const std::map<std::vector<bool>, char> &map)
{
    depth = 0;
    for (const auto &pair : map)
    {
        if (pair.first.size() > depth) depth = static_cast<int>(pair.first.size());
    }

    if (depth > 13) depth = 13;

    int size = static_cast<int>(std::pow(2, depth + 1));
    size -= 2;

    keys = new char[size];

    branchChar = 0;
    for (const auto &pair : map)
    {
        if (pair.second != branchChar) break;
        branchChar++;
    }

    // Fill keys with branchChar, concurrent for loop
    #pragma omp parallel for num_threads(2)
    for (int i = 0; i < size; i++) keys[i] = static_cast<char>(branchChar);

    for (const auto &pair : map)
    {
        if (pair.first.size() > 13)
        {
            overflowMap[pair.first] = pair.second;
            continue;
        }
        int index = 0;
        for (bool i : pair.first)
        {
            if (i) index = index * 2 + 2;
            else index = index * 2 + 1;
        }
        if (index >= size) throw std::runtime_error("Yo WTF Stack Smash");
        keys[index] = pair.second;
    }
}


CharHeap::~CharHeap()
{
    delete[] keys;
}


std::optional<char> CharHeap::getChar(const std::vector<bool> &key)
{
    if (key.size() > 13)
    {
        if (overflowMap.find(key) == overflowMap.end())
            return std::nullopt;

        return overflowMap[key];
    }

    int index = 0;
    for (bool i : key)
    {
        if (i) index = index * 2 + 2;
        else index = index * 2 + 1;
    }
    if (keys[index] == branchChar) return std::nullopt;
    return keys[index];
}


std::optional<char> CharHeap::operator[](const std::vector<bool> &key)
{
    return getChar(key);
}
