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
     * @param map A const map of vectors of booleans and their corresponding characters passed by reference
     */
    explicit CharHeap(const std::map<std::vector<bool>, char> &map);

    /**
     * @brief Destructor for the CharHeap class, frees the keys
     */
    ~CharHeap();

    /**
     * @brief Returns the character corresponding to a path
     *
     * @param key The path to get the character of
     * @return An empty optional if the path is not found, otherwise the character corresponding to the path
     */
    std::optional<char> getChar(const std::vector<bool> &key);

    /**
     * @brief Returns the path corresponding to a character
     *
     * @param key The path to get the character of
     * @return An empty optional if the path is not found, otherwise the character corresponding to the path
     */
    std::optional<char> operator[](const std::vector<bool> &key);

private:

    int depth; // The depth of the keys array
    char *keys; // The keys that are stored in the binary tree
    unsigned char branchChar = 0; // The character used to indicate that a key doesn't have a value
    std::map<std::vector<bool>, char> overflowMap; // Keys with a depth greater than 13 that aren't stored in the keys array
};


