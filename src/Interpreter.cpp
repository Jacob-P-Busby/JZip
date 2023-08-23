#include "Interpreter.h"


Interpreter::Interpreter(const std::map<char, std::vector<bool>> &charMap)
{
    // Reverse map
    std::map<std::vector<bool>, char> switchedMap;
    for (const auto &pair: charMap) {
        switchedMap[pair.second] = pair.first;
    }

    // Create CharHeap
    charHeap = new CharHeap(switchedMap);
}


Interpreter::~Interpreter()
{
    delete charHeap;
}

std::string Interpreter::decompress(const std::vector<bool> &in) const
{
    std::string returnString;
    std::vector<bool> tempVector;
    for (const bool &bit: in)
    {
        tempVector.push_back(bit);
        if (const auto maybeChar = charHeap->getChar(tempVector))
        {
            returnString += *maybeChar;
            tempVector.clear();
        }
    }

    return returnString;
}


std::string Interpreter::operator()(const std::vector<bool> &in) const
{
    return decompress(in);
}
