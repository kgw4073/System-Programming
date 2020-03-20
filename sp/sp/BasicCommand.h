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
extern void delete_trie(Trie* root);
extern void insert_trie(Trie* root, const char* key);
extern void find_trie(Trie* root, const char* key);


#endif