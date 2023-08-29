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

    // Fill keys with branchChar, indicating that the key is not in the map
    for (int i = 0; i < size; i++) keys[i] = static_cast<char>(branchChar);

    // Fill keys with values from map
    for (const auto &pair : map)
    {
        // If the key is too long, put it in the overflow map
        if (pair.first.size() > 13)
        {
            overflowMap[pair.first] = pair.second;
            continue;
        }

        // Convert key to index
        int index = 0;
        for (bool i : pair.first)
        {
            if (i) index = index * 2 + 2;
            else index = index * 2 + 1;
        }

        // If the index is out of bounds, should've been caught earlier and added to overflow map
        if (index >= size) throw std::runtime_error("Index out of bounds for CharHeap::keys");
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
