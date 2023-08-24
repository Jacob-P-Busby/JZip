#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "HuffmanTree.h"
#include "Interpreter.h"


/**
 * @brief Reads a file into a string
 *
 * @param in A const ifstream containing the file to read passed by reference
 * @return The contents of the file as a string
 */
[[nodiscard]] std::string slurp(std::ifstream &in)
{
    std::stringstream stringStream;
    stringStream << in.rdbuf();
    return stringStream.str();
}


/**
 * @brief Compresses a string using a map of characters and their paths, and outputs it to a file
 *
 * @details
 *
 *
 * @param in A const string to compress passed by reference
 * @param charMap A const map of characters and their paths used to compress the string passed by reference
 * @param out An ofstream to write the compressed data to
 *
 * @todo Change path to bitflags
 */
void compress(const std::string &in, const std::map<char, std::vector<bool>> &charMap, std::ofstream &out)
{
    for (const auto character : in)
        for (const auto &bit : charMap.at(character))
            out << bit;
}


/**
 * @brief Writes the dictionary to a file
 *
 * @details
 * The dictionary is written to a file in the following format:\n
 * 1 byte path length\n
 * n bytes path where n is the path length\n
 * 1 byte character\n
 * e.g. a(101) would be 0x03 FF 00 FF 61
 *
 * @param charMap The map of characters and their paths to write to the file
 * @param out The ofstream to write the dictionary to
 */
void writeDict(const std::map<char, std::vector<bool>> &charMap, std::ofstream &out)
{
    for (const auto &pair : charMap)
    {
        unsigned char pathLength = pair.second.size();
        out << pathLength;
        for (const auto &bit : pair.second)
        {
            out << bit;
        }
        out << pair.first;
    }
    out << '\0'; // Null terminator
}


/**
 * @brief reads the dictionary from a file
 *
 * @details
 * Assumes the dictionary is in the format specified in writeDict()\n
 * Will leave the file pointer at the first bit of the compressed data
 *
 * @param in the file to read from
 * @return the dictionary as a map of characters and their paths
 * @throws std::runtime_error if the file ends unexpectedly
 * @throws std::runtime_error if the file is not in the correct format
 *
 * @see writeDict()
 */
[[nodiscard]] std::map<char, std::vector<bool>> readDict(std::ifstream &in)
{
    std::map<char, std::vector<bool>> charMap;
    std::vector<bool> tempPath;
    while (true)
    {
        if (in.eof()) throw std::runtime_error("Unexpected EOF");
        unsigned char pathLength = in.get();
        if (pathLength == 0) break;

        for (int i = 0; i < pathLength; i++)
        {
            if (in.eof()) throw std::runtime_error("Unexpected EOF");
            bool bit = in.get();
            tempPath.push_back(bit);
        }

        if (in.eof()) throw std::runtime_error("Unexpected EOF");
        auto c = static_cast<char>(in.get());
        if (charMap.find(c) != charMap.end())
            throw std::runtime_error("Duplicate character in dictionary");

        charMap[c] = tempPath;
        tempPath.clear();
    }
    return charMap;
}


[[nodiscard]] std::string formatBytes(unsigned long bytes) {
    if (bytes < 1000) {
        return std::to_string(bytes) + " B";
    } else if (bytes < 1000 * 1000) {
        double kb = static_cast<double>(bytes) / 1000;
        return std::to_string(kb) + " KB";
    } else {
        double mb = static_cast<double>(bytes) / (1000 * 1000);
        return std::to_string(mb) + " MB";
    }
}

/**
 * Checks that there is 1 argument and that it is not a flag
 *
 * @param argc The count of arguments
 * @param argv The arguments
 * @return True if there is 1 argument and it is not a flag, false otherwise
 */
[[nodiscard]] bool validateArgs(int argc, char *argv[])
{
    if (argc != 2) return false; // Is there more than 1 argument? Not taking options/multiple files rn.
    if (argv[1][0] == '-') return false; // Is first argument a flag? Not using those rn.
    return true; // We all good
}


// Will give own file later, for now it lives here
/**
 * @brief Timer class used to measure program performance
 *
 * @details
 * Used to measure the time between the creation of the object and the call of getMilliseconds()\n
 * getMilliseconds() returns the time in milliseconds since the object was created
 *
 */
class Timer
{
public:
    Timer()
    {
        start = std::chrono::high_resolution_clock::now();
        recent = start;
    }

    /**
     * @brief Returns the time in microseconds since the object was created
     *
     * @return The time in milliseconds since the object was created
     */
    [[nodiscard]] long cumMicroseconds() const
    {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }


    /**
     * @brief Returns the time in microseconds since the last call of sectMicroseconds() or the object was created
     *
     * @return A long representing the time in microseconds since the last call of sectMicroseconds()
     *         or the object was created
     */
    [[nodiscard]] long sectMicroseconds()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto returnTime = std::chrono::duration_cast<std::chrono::microseconds>(end - recent).count();
        recent = end;
        return returnTime;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start; // Time when object was created
    std::chrono::time_point<std::chrono::high_resolution_clock> recent; // Time when getMilliseconds() was called
};


int main(int argc, char *argv[])
{
    Timer timer;

    // Ensure correct number of args
    if (!validateArgs(argc, argv)) {
        std::cout << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    // Check if file exists and is accessible
    std::ifstream inFile(argv[1], std::ios::binary);
    if (!inFile.good()) {
        std::cout << "File can't be read" << '\n';
        std::cout << "Make sure the file exists and is accessible" << '\n';
        return 1;
    }

    // Check if file is empty
    inFile.seekg(0, std::ios::end);
    if (inFile.tellg() == 0) {
        std::cout << "File is empty" << '\n';
        return 1;
    }

    // Check if file is too large
    inFile.seekg(0, std::ios::beg);
    if (inFile.tellg() > 1000 * 1000 * 20) {
        std::cout << "File is too large" << '\n';
        std::cout << "File must be less than 20MB" << '\n';
        return 1;
    }
    auto validityCheckTime = timer.sectMicroseconds();

    // Slurp and close file
    std::string input = slurp(inFile);
    inFile.close();
    auto slurpTime = timer.sectMicroseconds();

    // Ensure fie contains stuff to compress
    if (input.empty()) throw std::runtime_error("Empty file");

    HuffmanTree huffmanTree(input);
    auto huffmanTreeTime = timer.sectMicroseconds();

    std::map<char, std::vector<bool>> charMap = huffmanTree.getKeys();
    auto getKeysTime = timer.sectMicroseconds();

    // Create jzip filename
    std::string jzipFileName = argv[1];
    jzipFileName += ".jzip";

    std::ofstream outFile(jzipFileName, std::ios::binary);
    if (!outFile.good()) throw std::runtime_error("outFile is not good");
    auto outFileTime = timer.sectMicroseconds();

    writeDict(charMap, outFile);
    auto writeDictTime = timer.sectMicroseconds();

    // Write compressed data to file
    compress(input, charMap, outFile);
    auto compressTime = timer.sectMicroseconds();

    outFile.close();

    inFile.open(jzipFileName);

    // Ensure inFile is in good condition
    if (!inFile.good()) {
        throw std::runtime_error("inFile is not good");
    }
    auto inFileTime = timer.sectMicroseconds();

    std::map<char, std::vector<bool>> newCharMap = readDict(inFile);
    auto readDictTime = timer.sectMicroseconds();

    for (const auto &pair : charMap)
    {
        assert(pair.second.size() == newCharMap[pair.first].size());
        for (int i = 0; i < pair.second.size(); i++)
        assert(pair.second[i] == charMap[pair.first][i]);
    }
    auto dictAssertTime = timer.sectMicroseconds();

    Interpreter interpreter(charMap);
    std::string output = interpreter.decompress(inFile);
    std::cout << output.size() << '\n';
    std::cout << output << '\n';
    auto interpreterTime = timer.sectMicroseconds();


    std::cout << "validityCheck: " << validityCheckTime << " microseconds" << '\n';
    std::cout << "slurp: " << slurpTime << " microseconds" << '\n';
    std::cout << "huffmanTree: " << huffmanTreeTime << " microseconds" << '\n';
    std::cout << "getKeys: " << getKeysTime << " microseconds" << '\n';
    std::cout << "outFile: " << outFileTime << " microseconds" << '\n';
    std::cout << "writeDict: " << writeDictTime << " microseconds" << '\n';
    std::cout << "compress: " << compressTime << " microseconds" << '\n';
    std::cout << "inFile: " << inFileTime << " microseconds" << '\n';
    std::cout << "readDict: " << readDictTime << " microseconds" << '\n';
    std::cout << "dictAssertTime: " << dictAssertTime << " microseconds" << '\n';
    std::cout << "interpreter: " << interpreterTime << " microseconds" << '\n';
    std::cout << "cumTime: " << timer.cumMicroseconds() << " microseconds" << "\n\n";

    std::cout << "Original file size: " << formatBytes(std::filesystem::file_size(argv[1])) << '\n';
    std::cout << "Compressed file size: " << formatBytes(std::filesystem::file_size(jzipFileName)) << '\n';

    std::cout << "All done :)" << std::endl;

    return 0;
}
