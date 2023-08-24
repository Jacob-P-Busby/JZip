#pragma once


#include <cmath>
#include <map>
#include <stdexcept>
#include <vector>
#include <optional>


class CharHeap {
public:

    /**
     * @brief Constructor for the CharHeap class
     *
     * @param map The map of vectors of booleans and their corresponding characters passed by reference
     */
    explicit CharHeap(const std::map<std::vector<bool>, char> &map);

    ~CharHeap();

    /**
     * @brief Returns the character corresponding to a vector of booleans
     *
     * @param key The key to search for
     * @return Empty optional if key not found, otherwise the character corresponding to the key
     */
    std::optional<char> getChar(const std::vector<bool> &key);

    /**
     * @brief Returns the character corresponding to a vector of booleans
     *
     * @param key The key to search for
     * @return Empty optional if key not found, otherwise the character corresponding to the key
     */
    std::optional<char> operator[](const std::vector<bool> &key);

private:

    int depth; // The depth of the tree, maxes out at 13 and then overflows into a map
    char *keys; // The characters stored in the tree
    unsigned char branchChar = 0; // The character used to represent a branch node
    std::map<std::vector<bool>, char> overflowMap; // The map used to store characters when the tree overflows
};


