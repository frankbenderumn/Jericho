#ifndef JERICHO_STRUCTURE_TRIE_H_
#define JERICHO_STRUCTURE_TRIE_H_

#include <string>

#include "server/defs.h"

class TrieNode {
    std::string _name;
    TrieNode* _left = nullptr;
    TrieNode* _right = nullptr;
  public:
    TrieNode(std::string name) : _name(name) { PCREATE; }
    ~TrieNode() { PDESTROY; }
};

class Trie {
    TrieNode* _root;
  public:
    Trie() { PCREATE; }
    ~Trie() { PDESTROY; }
};

#endif