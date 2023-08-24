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

    int depth;
    char *keys;
    unsigned char branchChar = 0;
    std::map<std::vector<bool>, char> overflowMap;
};


