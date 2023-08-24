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


std::string Interpreter::decompress(std::ifstream &in) const
{
    std::string returnString;
    std::vector<bool> key;
    auto current = in.tellg();
    in.seekg(0, std::ios::end);
    auto end = in.tellg();
    in.seekg(current);

    while (true)
    {
        char c;
        in >> c;
        if (in.tellg() == end) break;

        if (c == '1') key.push_back(true);
        else if (c == '0') key.push_back(false);
        else throw std::runtime_error("Invalid character in compressed file");

        if (charHeap->getChar(key).has_value())
        {
            returnString += charHeap->getChar(key).value();
            key.clear();
        }
    }
    return returnString;
}


std::string Interpreter::operator()(std::ifstream &in) const
{
    return decompress(in);
}
