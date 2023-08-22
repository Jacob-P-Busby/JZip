#pragma once

#include <cmath>
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
     * @param in A const vector of booleans to decompress passed by reference
     * @return The decompressed string
     */
    [[nodiscard]] std::string decompress(const std::vector<bool> &in) const;

    [[nodiscard]] std::string operator()(const std::vector<bool> &in) const;

private:
    CharHeap *charHeap; // Pointer to the CharHeap used to store the characters
};
