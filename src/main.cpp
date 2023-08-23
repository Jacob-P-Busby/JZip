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
 * @brief Writes out the vector of booleans to the ofstream
 *
 * @details
 * The vector is written out bit by bit\n
 * If the vector is not a multiple of 8, the last byte will be padded with undefined bits
 *
 * @param toWrite The data to be written out
 * @param out The ofstream to write to
 */
void writeOut(std::vector<bool> &toWrite, std::ofstream &out)
{
    int bitCount = 0;
    unsigned char byte = 0;
    for (const auto &bit : toWrite)
    {
        byte = (byte << 1) | bit;
        bitCount++;
        if (bitCount == 8)
        {
            out << byte;
            byte = 0;
            bitCount = 0;
        }
    }
    if (bitCount != 0)
    {
        byte = byte << (8 - bitCount);
        out << byte;
    }
}


/**
 * @brief Compresses a string using a map of characters and their paths, and outputs it to a file in 10kb chunks
 *
 * @details
 * This function assumes the map is valid,
 * the string is not empty,
 * and the dict has already been written to the file\n
 * The compressed data is written out in the following format:\n
 * 1 byte counting the bits to be ignored at the of the chunk\n
 * 9999 bytes of compressed data, including undefined trailing bits\n
 * The chunks can be decompressed in parallel, allowing for faster decompression using multithreading\n
 * The last chunk will be padded with with undefined trailing bits to the nearest byte, these will be counted
 * by the first byte of the chunk, and ignored when decompressing
 *
 * @param in A const string to compress passed by reference
 * @param charMap A const map of characters and their paths used to compress the string passed by reference
 * @param out An ofstream to write the compressed data to
 */
void compress(const std::string &in, const std::map<char, std::vector<bool>> &charMap, std::ofstream &out)
{
    std::vector<bool> compressedData;
    std::stringstream stringStream(in);

    // Reserve space for trailing bits
    for (int i = 0; i < 8; i++)
        compressedData.push_back(false);


    while (true)
    {
        if (stringStream.eof())
        {
            if (compressedData.size() % 8 != 0)
            {
                unsigned char trailingBits = 0;
                while (compressedData.size() % 8 != 0)
                {
                    compressedData.push_back(false);
                    trailingBits++;
                }
                for (int i = 0; i < 8; i++)
                    compressedData[i] = (trailingBits >> (7 - i)) & 1;
            }
            writeOut(compressedData, out);
            break;
        }

        char c;
        stringStream >> c;
        for (const auto &bit : charMap.at(c))
        {
            compressedData.push_back(bit);
        }

        if (compressedData.size() > 80000) // 10KB in bits
        {
            unsigned char trailingBits = 0;
            for (int i = 0; i < charMap.at(c).size(); i++)
                compressedData.pop_back();
            while (compressedData.size() != 80000)
            {
                compressedData.push_back(false);
                trailingBits++;
            }
            for (int i = 0; i < 8; i++)
                compressedData[i] = (trailingBits >> (7 - i)) & 1;
            writeOut(compressedData, out);
            compressedData.clear();
        }
    }
}


// TODO: Change path to bitflags
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


[[nodiscard]] std::map<char, std::vector<bool>> readDict(std::ifstream &in)
{
    std::map<char, std::vector<bool>> charMap;
    std::vector<bool> tempPath;
    while (true)
    {
        if (in.eof()) throw std::runtime_error("Unexpected EOF");
        unsigned char pathLength;
        pathLength = in.get();
        if (pathLength == 0) break;

        for (int i = 0; i < pathLength; i++)
        {
            if (in.eof()) throw std::runtime_error("Unexpected EOF");
            bool bit;
            bit = in.get();
            tempPath.push_back(bit);
        }

        if (in.eof()) throw std::runtime_error("Unexpected EOF");
        char c;
        c = static_cast<char>(in.get());
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
    std::cout << "Here\n";
    auto readDictTime = timer.sectMicroseconds();

    for (const auto &pair : charMap)
    {
        assert(pair.second.size() == newCharMap[pair.first].size());
        for (int i = 0; i < pair.second.size(); i++)
        assert(pair.second[i] == charMap[pair.first][i]);
    }
    auto assertTime = timer.sectMicroseconds();

    std::cout << "validityCheck: " << validityCheckTime << " microseconds" << '\n';
    std::cout << "slurp: " << slurpTime << " microseconds" << '\n';
    std::cout << "huffmanTree: " << huffmanTreeTime << " microseconds" << '\n';
    std::cout << "getKeys: " << getKeysTime << " microseconds" << '\n';
    std::cout << "outFile: " << outFileTime << " microseconds" << '\n';
    std::cout << "writeDict: " << writeDictTime << " microseconds" << '\n';
    std::cout << "compress: " << compressTime << " microseconds" << '\n';
    std::cout << "inFile: " << inFileTime << " microseconds" << '\n';
    std::cout << "readDict: " << readDictTime << " microseconds" << '\n';
    std::cout << "assertTime: " << assertTime << " microseconds" << '\n';
    std::cout << "cumTime: " << timer.cumMicroseconds() << " microseconds" << "\n\n";
    std::cout << "Original file size: " << formatBytes(std::filesystem::file_size(argv[1])) << '\n';
    std::cout << "Compressed file size: " << formatBytes(std::filesystem::file_size(jzipFileName)) << '\n';

    std::cout << "All done :)" << '\n';

    return 0;
}
