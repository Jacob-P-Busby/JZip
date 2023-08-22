#include <cassert>
#include <chrono>
#include <functional>
#include <fstream>
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

/**
 * @brief Converts a map of characters and boolean vectors into a single boolean vector
 *
 * @details
 * This uses the following format:\n
 * 8-bit char path length + n bit path rounded up to the nearest byte + 8-bit ASCII character\n
 * e.g. 3101a = 00000011 10100000 01100001\n
 * null (00000000) where path length would be, indicates end of dict and start of content\n
 * Unused bits are undefined, don't rely on them being 0
 *
 * @param charMap a const map of characters and their paths passed by reference
 * @return A vector of booleans representing the dict, using the format described above
 * @throws std::runtime_error if a path is longer than 255 bits
 *
 * @todo Remove byte remove byte rounding, increasing compaction, low priority but would be nice
 */
[[nodiscard]] std::vector<bool> vectorDict(const std::map<char, std::vector<bool>> &charMap)
{
    std::vector<bool> returnVector;
    for (const auto &key : charMap)
    {
        std::vector<bool> appendVector;

        // Path length
        if (key.second.size() > 255) throw std::runtime_error("Path length too long");
        auto pathLength = static_cast<unsigned char>(key.second.size());
        for (int i = 7; i >= 0; i--)
        {
            returnVector.push_back((pathLength >> i) & 1);
        }

        // Path
        for (const auto &bit : key.second)
        {
            returnVector.push_back(bit);
        }
        // Blank bits to nearest 8th
        for (int i = 0; (i < (8 - (key.second.size() % 8))) && key.second.size() % 8 != 0; i++)
        {
            returnVector.push_back(false);
        }

        // Character
        for (int i = 7; i >= 0; i--)
        {
            returnVector.push_back((key.first >> i) & 1);
        }
    }
    // Null character to indicate end of dict
    for (int i = 0; i < 8; i++)
    {
        returnVector.push_back(false);
    }

    return returnVector;
}


/**
 * @brief Interprets a boolean vector dict into a map of characters and their paths
 *
 * @details
 * This assumes the dict is in the format described in vectorDict()
 *
 * @param binDict A const vector of booleans representing the dict passed by reference\n
 * <bold>THIS WILL BE LEFT AT THE END OF THE DICT AFTER THE NULL TERMINATOR</bold>
 * @return A map of characters and their paths
 * @throws std::runtime_error Thrown if the dict comes to an unexpected end
 */
[[nodiscard]] std::map<char, std::vector<bool>> interpretDict(std::ifstream &in)
{
    std::map<char, std::vector<bool>> returnMap;
    while (true)
    {
        unsigned char pathLength = 0;
        in >> pathLength;
        if (pathLength == 0) break;

        std::vector<bool> path;
        unsigned char bits;
        for (int i = 0; i < pathLength; i++)
        {
            if (i % 8 == 0) // Is a new byte needed?
                in >> bits;

            else
                path.push_back(bits >> (7 - (i % 8)) & 1); // Add new bit to path
        }

        // Add path to map
        char c;
        in >> c;
        returnMap[c] = path;
    }
    return returnMap;
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


/**
 * @brief Writes the dict and compressed data to a file
 *
 * @details
 * This is thread safe, as long as ofstream isn't used, and is closed after the thread is joined\n\n
 *
 * binDict is written out as is, not including the null character appended at the end.\n
 *
 * compressedData is written out in the following format:\n
 * 1 byte counting the trailing bits at the end\n
 * 9999 bytes of compressed data, including undefined trailing bits\n
 * This is to create nice 10kb thread safe chunks to allow parallel decompression
 *
 * @param binDict The dictionary in the format described in vectorDict()
 * @param compressedData The data from the infile compressed using the dict
 * @param outFile The ofstream to write to
 *
 * @see vectorDict()
 *
 * @todo Break up into 10kb chunks for parallel decompression, high priority
 * @todo Clean up, shit looks like it came from an Italian restaurant with the amount of spaghetti here
 */
void writeOutDict(const std::vector<bool> &binDict, std::ofstream &outFile)
{
    // Write dict to file
    unsigned char byte = 0;
    int bitCount = 0;
    for (const auto &bit : binDict)
    {
        byte = (byte << 1) | bit;
        bitCount++;
        if (bitCount == 8)
        {
            outFile << byte;
            byte = 0;
            bitCount = 0;
        }
    }

    // Write trailing bits, although there shouldn't be any
    while (bitCount != 0)
    {
        byte = (byte << 1);
        bitCount++;
        if (bitCount == 8)
        {
            outFile << byte;
            byte = 0;
            bitCount = 0;
        }
    }

    // null character to switch from dict to data
    outFile << '\0';
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

    std::vector<bool> dict = vectorDict(charMap);
    auto vectorDictTime = timer.sectMicroseconds();

    // Create jzip filename
    std::string jzipFileName = argv[1];
    jzipFileName += ".jzip";

    // Write to file, separate thread to avoid IO blocking
    std::ofstream outFile(jzipFileName, std::ios::binary);
    writeOutDict(dict, std::ref(outFile));
    auto writeOutDictTime = timer.sectMicroseconds();

    // Write compressed data to file
    compress(input, charMap, outFile);
    auto compressTime = timer.sectMicroseconds();

    outFile.close();

    inFile.open(jzipFileName);

    // Ensure inFile is in good condition
    if (!inFile.good())
        throw std::runtime_error("inFile is not good");

    std::map<char, std::vector<bool>> newCharMap = interpretDict(inFile);
    auto interpretDictTime = timer.sectMicroseconds();

    assert(newCharMap == charMap);

    std::cout << "validityCheck: " << validityCheckTime << " microseconds" << '\n';
    std::cout << "slurp: " << slurpTime << " microseconds" << '\n';
    std::cout << "huffmanTree: " << huffmanTreeTime << " microseconds" << '\n';
    std::cout << "getKeys: " << getKeysTime << " microseconds" << '\n';
    std::cout << "vectorDict: " << vectorDictTime << " microseconds" << '\n';
    std::cout << "writeOutDict: " << writeOutDictTime << " microseconds" << '\n';
    std::cout << "compress: " << compressTime << " microseconds" << '\n';
    std::cout << "interpretDict: " << interpretDictTime << " microseconds" << '\n';
    std::cout << "cumTime: " << timer.cumMicroseconds() << " microseconds" << '\n';

    std::cout << "All done :)" << std::endl;

    return 0;
}
