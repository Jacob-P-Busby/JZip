#pragma once

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>
#include <map>
#include <vector>

#include "CharHeap.h"


class Interpreter {
public:
    /**
     * @brief Constructor for the Interpreter class
     *
     * @param charMap A const map of vectors of booleans and their corresponding characters passed by reference
     */
    explicit Interpreter(const std::map<char, std::vector<bool>> &charMap);

    ~Interpreter();

/**
     * @brief Decompresses a vector of booleans using a CharHeap
     *
     * @param in An ifstream containing the file to decompress
     * @return The decompressed string
     */
    [[nodiscard]] std::string decompress(std::ifstream &in) const;

    /**
     * @brief Decompresses a vector of booleans using a CharHeap
     *
     * @param in A ifstream containing the file to decompress
     * @return The decompressed string
     */
    [[nodiscard]] std::string operator()(std::ifstream &in) const;

private:
    CharHeap *charHeap; // Pointer to the CharHeap used to store the characters
};
