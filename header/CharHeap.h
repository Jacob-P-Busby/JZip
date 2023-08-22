#pragma once


#include <cmath>
#include <map>
#include <stdexcept>
#include <vector>
#include <optional>


class CharHeap {
public:

    explicit CharHeap(const std::map<std::vector<bool>, char> &map);

    ~CharHeap();

    std::optional<char> getChar(const std::vector<bool> &key);

    std::optional<char> operator[](const std::vector<bool> &key);

private:

    int depth;
    char *keys;
    unsigned char branchChar = 0;
    std::map<std::vector<bool>, char> overflowMap;
};


