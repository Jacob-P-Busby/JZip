#include "HuffmanTree.h"


HuffmanTree::HuffmanTree(const std::string &input) {
    auto charMap = countChars(input); // Map of characters and their frequencies
    std::priority_queue<Node *, std::vector<Node *>, CompareValue> nodeQueue; // Priority queue of nodes

    // Create leaf nodes
    for (auto &character: charMap) {
        nodeQueue.push(new Node(character.first, character.second));
    }

    // Merge leaf nodes into tree
    while (nodeQueue.size() > 1) {
        Node *pLeft = nodeQueue.top(); // Left child
        nodeQueue.pop();
        Node *pRight = nodeQueue.top(); // Right child
        nodeQueue.pop();
        nodeQueue.push(new Node(pLeft, pRight)); // New branch node holding children
    }

    // Set root node
    pRoot = nodeQueue.top();
}


HuffmanTree::~HuffmanTree() {
    delete pRoot; // Trigger recursive destructor of Node, ensuring all Nodes are deleted
}


std::map<char, int> HuffmanTree::countChars(const std::string &input) {
    std::map<char, int> charMap;
    for (char c: input) {
        charMap[c]++;
    }
    return charMap;
}


std::map<char, std::vector<bool>> HuffmanTree::getKeys() const {
    return pRoot->getKeys(); // Trigger recursive getKeys() of Node, ensuring all Nodes are visited
}


HuffmanTree::Node::Node(HuffmanTree::Node *pLChild, HuffmanTree::Node *pRChild) {
    this->freq = pLChild->freq + pRChild->freq;
    this->pLeft = pLChild;
    this->pRight = pRChild;
}


HuffmanTree::Node::Node(char c, long long int freq) {
    this->c = c;
    this->freq = freq;
}


HuffmanTree::Node::~Node() {
    if (!pLeft) return; // Leaf node
    delete pLeft;
    delete pRight;
}


// NOLINTNEXTLINE(*-no-recursion)
std::map<char, std::vector<bool>> HuffmanTree::Node::getKeys() const {
    static std::vector<bool> path; // Path used to get to this node, left = 0, right = 1, shared by all nodes
    if (pLeft) // Am I a branch node?
    {
        // Get maps from children
        path.push_back(false); // 0
        std::map<char, std::vector<bool>> leftMap = pLeft->getKeys();
        path.push_back(true); // 1
        std::map<char, std::vector<bool>> rightMap = pRight->getKeys();

        // Add all characters from right map to left map
        for (auto &character: rightMap) {
            leftMap[character.first] = character.second;
        }

        // Pop bit from path if not root node
        if (!(path.empty())) path.pop_back();
        return leftMap;
    }

    // Create map with a single entry containing the character and its path
    std::map<char, std::vector<bool>> returnMap;
    returnMap[c] = std::vector<bool>();
    for (std::_Bit_reference bit: path) {
        returnMap[c].push_back(bit);
    }
    path.pop_back();
    return returnMap;
}


bool HuffmanTree::CompareValue::operator()(const HuffmanTree::Node *a, const HuffmanTree::Node *b) const {
    return a->freq > b->freq;
}
