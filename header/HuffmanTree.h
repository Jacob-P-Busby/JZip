#pragma once


#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <map>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>

/**
 * @brief HuffmanTree class used to generate a tree of characters and their paths
 *
 * @details
 * Uses a priority queue to merge nodes into a tree\n
 * The tree is traversed to generate a map of characters and their paths\n
 * The map is returned by getKeys()\n
 * The tree is deleted by the destructor, ensuring all nodes are deleted
 *
 * @see Node
 */
class HuffmanTree
{
public:
    /**
     * @brief Constructor for HuffmanTree
     *
     * @details
     * Generates a map of characters and their frequencies\n
     * Creates leaf nodes for each character and adds them to a priority queue\n
     * Merges the leaf nodes into a tree by popping the two lowest frequency nodes and pushing a new branch node\n
     * Sets the root node to the last node in the queue
     *
     * @param input The string to generate the tree from
     */
    explicit HuffmanTree(const std::string &input);

    /**
     * @brief Destructor for HuffmanTree
     *
     * @details
     * Deletes the root node, triggering its destructor\n
     * This ensures all nodes are deleted
     *
     * @see HuffmanTree::Node::~Node()
     */
    ~HuffmanTree();

    /**
     * @brief Returns a map of characters and their paths
     *
     * @details
     * Calls getKeys() on the root node, triggering its recursive getKeys()\n
     * This ensures all nodes are visited
     *
     * @return A map of characters and their paths
     *
     * @see HuffmanTree::Node::getKeys()
     */
    [[nodiscard]] std::map<char, std::vector<bool>> getKeys() const;

private:
    /**
     * @brief Counts the number of occurrences of each character in a string
     *
     * @param input A const string to count the characters of passed by reference
     * @return The amount of times each character occurs in the string as a map
     */
    static std::map<char, int> countChars(const std::string &input);


    /**
     * @brief Function class used to compare two nodes by their frequency
     * @details
     * Used by std::priority_queue to sort nodes by their frequency in ascending order
     *
     * @see HuffmanTree::CompareValue::operator()
     */
    class Node
    {
    public:
        char      c      = 0; // Character that the node represents
        long long freq   = 0; // The amount of times the character occurs
        Node     *pLeft  = nullptr; // Left child, nullptr if leaf node
        Node     *pRight = nullptr; // Right child, nullptr if leaf node

        /**
         * @brief Constructor for branch nodes
         *
         * @details
         * Sets the left and right children to the values passed in\n
         * Sets the frequency to the sum of the children's frequencies\n
         * c is set to 0 since it is not used by branch nodes
         *
         * @param pLChild The child to set as the left child (0)
         * @param pRChild The child to set as the right child (1)
         */
        Node(Node *pLChild, Node *pRChild);

        /**
         * @brief Constructor for leaf nodes
         *
         * @details
         * Sets the character and frequency to the values passed in\n
         * Sets the left and right children to nullptr since leaf nodes have no children
         *
         * @param c The character to set
         * @param freq The frequency to set
         */
        Node(char c, long long freq);

        /**
         * @brief Destructor for branch nodes
         *
         * @details
         * Deletes the left and right children, triggering their destructors\n
         * This ensures all nodes are deleted
         */
        ~Node();

        /**
         * @brief Returns a map of characters and their paths, including grandchildren
         *
         * @details
         * Will remove recursion eventually\n
         * Shouldn't overflow since max depth is 257 (256 possible character + root)\n
         * Tested and overflowed at ~10000 depth\n
         *
         * @return A map of characters and their paths, expressed as a vector of booleans
         *
         * @todo Remove recursion, stack overflow isn't possible but it's still bad practice
         */
        [[nodiscard]] std::map<char, std::vector<bool>> getKeys() const;
    };


    /**
     * @brief Function class used to compare two nodes by their frequency
     * @details
     * Used by std::priority_queue to sort nodes by their frequency in ascending order
     *
     * @see HuffmanTree::CompareValue::operator()
     */
    class CompareValue
    {
    public:
        /**
         * @brief Compares two nodes by their frequency
         *
         * @param a the first node to compare
         * @param b the second node to compare
         * @return true if a's frequency is greater than b's frequency
         * @see CompareValue
         */
        [[nodiscard]] bool operator()(const Node *a, const Node *b) const;
    };
    Node *pRoot = nullptr; // Root node of tree
};