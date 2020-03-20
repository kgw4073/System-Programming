
#ifndef _BASIC_COMMAND_H
#define _BASIC_COMMAND_H
#include "20150514.h"
#include "command.h"

typedef struct Trie {
	bool terminal;
	struct Trie* child[MAX_ALPHA];
} Trie;

Trie* root;



extern Trie* getNewTrieNode();
extern void deleteTrie(Trie* root);
extern void insertTrie(Trie* root, char* key);
extern bool searchTrie(Trie* root, char* key);


#endif