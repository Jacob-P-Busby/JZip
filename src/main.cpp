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
void compressOut(const std::string &in, const std::map<char, std::vector<bool>> &charMap, std::ofstream &out)
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
        for (const auto &bit : pair.second) // Write out bit
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
            unsigned char bit = in.get();
            if (bit == '0') tempPath.push_back(false);
            else if (bit == '1') tempPath.push_back(true);
            else throw std::runtime_error("Invalid bit");
        }

        if (in.eof()) throw std::runtime_error("Unexpected EOF");
        auto c = static_cast<char>(in.get()); // The character is the byte next to the path
        if (charMap.find(c) != charMap.end())
            throw std::runtime_error("Duplicate character in dictionary");

        charMap[c] = tempPath;
        tempPath.clear();
    }
    return charMap;
}


/**
 * @brief Formats bytes into a human readable format
 *
 * @param bytes The number of bytes to format
 * @return The bytes formatted as a string with the appropriate unit
 */
[[nodiscard]] std::string formatBytes(unsigned long bytes) {
    if (bytes < 1000)
        return std::to_string(bytes) + " B";

    if (bytes < 1000 * 1000)
    {
        double kb = static_cast<double>(bytes) / 1000;
        return std::to_string(kb) + " KB";
    }

    double mb = static_cast<double>(bytes) / (1000 * 1000);
    return std::to_string(mb) + " MB";
}


std::map<char, std::vector<bool>> compress(const std::string &file)
{
    // Check if file exists and is accessible
    std::ifstream inFile(file, std::ios::binary);
    if (!inFile.good()) {
        std::cout << "File can't be read" << '\n';
        std::cout << "Make sure the file exists and is accessible" << '\n';
        throw std::runtime_error("File can't be read");
    }

    // Check if file is empty
    inFile.seekg(0, std::ios::end);
    if (inFile.tellg() == 0) {
        std::cout << "File is empty" << '\n';
        throw std::runtime_error("File is empty");
    }

    // Check if file is too large
    inFile.seekg(0, std::ios::beg);
    if (inFile.tellg() > 1000 * 1000 * 20) {
        std::cout << "File is too large" << '\n';
        std::cout << "File must be less than 20MB" << '\n';
        throw std::runtime_error("File is too large");
    }

    // Slurp and close file
    std::string input = slurp(inFile);
    inFile.close();

    // Ensure fie contains stuff to compress
    if (input.empty()) throw std::runtime_error("Empty file");

    HuffmanTree huffmanTree(input);

    std::map<char, std::vector<bool>> charMap = huffmanTree.getKeys();

    // Create jzip filename
    std::string jzipFileName = file;
    jzipFileName += ".jzip";

    std::ofstream outFile(jzipFileName, std::ios::binary);
    if (!outFile.good()) throw std::runtime_error("outFile is not good");

    writeDict(charMap, outFile);

    // Write compressed data to file
    compressOut(input, charMap, outFile);

    outFile.close();

    return charMap;
}


/**
 * @brief Decompresses a file, saving the output to a file
 *
 * @param fileName The name/path of the file to decompress
 * @param outFileName The name/path of the file to save the output to
 */
void inflate(const std::string &fileName, const std::string &outFileName)
{
    std::ifstream inFile(fileName, std::ios::binary);

    // Ensure inFile is in good condition
    if (!inFile.good()) {
        throw std::runtime_error("inFile is not good");
    }

    std::map<char, std::vector<bool>> newCharMap = readDict(inFile);

    // Decompress the body
    Interpreter interpreter(newCharMap);
    std::string output = interpreter.decompress(inFile);

    std::ofstream outFile(outFileName);

    // Ensure outFile is in good condition
    if (!outFile.good()) {
        throw std::runtime_error("outFile is not good");
    }

    outFile << output;
    outFile.close();
}

// Will give own file later, for now it lives here
/**
 * @brief Timer class used to measure program performance
 *
 * @details
 * Used to measure the time between the creation of the object and the call of getMicroseconds()\n
 * Can also be used to measure the time between calls of sectMicroseconds()\n
 */
class Timer
{
public:
    Timer()
    {
        start = std::chrono::high_resolution_clock::now();
        sect = start;
    }

    /**
     * @brief Returns the time in microseconds since the object was created
     *
     * @return The time in microseconds since the object was created
     */
    [[nodiscard]] long cumMicroseconds() const
    {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    /**
     * @brief Returns the time in microseconds since the last call of sectMicroseconds()
     *
     * @returns The time in microseconds since the last call of sectMicroseconds()
     */
    [[nodiscard]] long sectMicroseconds()
    {
        auto end = std::chrono::high_resolution_clock::now();
        long time = std::chrono::duration_cast<std::chrono::microseconds>(end - sect).count();
        sect = end;
        return time;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start; // Time when object was created
    std::chrono::time_point<std::chrono::high_resolution_clock> sect; // Time when sectMicroseconds() was called
};


int test(const std::string &file)
{
    std::cout << "====================\n";
    std::cout << "Testing " << file << '\n';
    std::cout << "====================\n";

    std::cout << "Compressing...\n";
    compress(file);
    std::cout << "Compressed\n\n";

    // Ensure fie contains stuff to compress
    std::string jzipFileName = file;
    jzipFileName += ".jzip";

    std::cout << "Inflating...\n";
    inflate(jzipFileName, file + ".out");
    std::cout << "Inflated\n\n";

    std::cout << "Original file size: " << formatBytes(std::filesystem::file_size(file)) << '\n';
    std::cout << "Compressed file size: " << formatBytes(std::filesystem::file_size(jzipFileName)) << "\n";

    return 0;
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Invalid usage\nSee -h for more information" << std::endl;
        return 1;
    }

    std::string arg1 = argv[1];

    if (arg1 == "-h" || arg1 == "--help")
    {
        std::cout << "Usage:\n";
        std::cout << "jzip <flags>\n";
        std::cout << "jzip <command> [<file> ...]\n\n";

        std::cout << "Flags:\n";
        std::cout << "-h --help\n";
        std::cout << "    Display this help message\n";
        std::cout << "-t --test\n";
        std::cout << "    Run tests, assuming it is executed in the working directory provided\n\n";

        std::cout << "Example: jzip file.txt\n";
        std::cout << "Example: jzip -t\n";
        return 0;
    }

    if (arg1 == "-t" || arg1 == "--test")
    {
        Timer timer;
        std::cout << "Running tests...\n\n";

        test("bee.txt");
        std::cout << "Test time: " << timer.sectMicroseconds() << " microseconds\n\n";
        test("ecoli.txt");
        std::cout << "Test time: " << timer.sectMicroseconds() << " microseconds\n\n";
        test("bible.txt");
        std::cout << "Test time: " << timer.sectMicroseconds() << " microseconds\n\n";

        std::cout << "====================\n";
        std::cout << "All tests completed\n";
        std::cout << "====================\n\n";

        std::cout << "Total test time: " << timer.cumMicroseconds() << " microseconds\n";
    }
    else
    {
        std::cout << "Actual usage isn't implemented yet :/\n";
        return 1;
    }


    return 0;
}
