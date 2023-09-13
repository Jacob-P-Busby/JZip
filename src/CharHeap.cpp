#include "CharHeap.h"


CharHeap::CharHeap(const std::map<std::vector<bool>, char> &map)
{
    // Find the longest key in the map
    depth = 0;
    for (const auto &pair : map)
    {
        if (static_cast<int>(pair.first.size()) > depth) depth = static_cast<int>(pair.first.size());
    }

    if (depth > 13) depth = 13; // Max depth is 13, anything longer is put in the overflow map

    int size = static_cast<int>(std::pow(2, depth + 1));
    size -= 2;

    keys = new char[size];

    // Sift through map to find the first character that isn't in the map
    // TODO: Handle every character being used
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
        keys[index] = pair.second;
    }
}


CharHeap::~CharHeap()
{
    delete[] keys;
}


std::optional<char> CharHeap::getChar(const std::vector<bool> &key)
{
    if (key.size() > 13) // If the key is too long, check the overflow map
    {
        if (overflowMap.find(key) == overflowMap.end())
            return std::nullopt; // If the key isn't in the overflow map, return an empty optional

        return overflowMap[key];
    }

    int index = 0;
    for (bool i : key) // Convert key to index
    {
        if (i) index = index * 2 + 2;
        else index = index * 2 + 1;
    }
    if (keys[index] == branchChar) return std::nullopt; // If the key isn't in the CharHeap, return an empty optional
    return keys[index]; // Return the value at the index, which is the character corresponding to the key
}


std::optional<char> CharHeap::operator[](const std::vector<bool> &key)
{
    return getChar(key);
}
