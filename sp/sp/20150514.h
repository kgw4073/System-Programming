
#ifndef _MAIN_HEADER_H
#define _MAIN_HEADER_H
#define MAX_COMMAND_SIZE 256
#define MAX_MEMORY_LINE (1 << 16)
#define MAX_BYTES_LINE (1 << 4)
#define MAX_HASH_SIZE 20
#define bool int
#define true 1
#define false 0
#define FILE_ERROR(); { fprintf(stderr, "cannot find the file...\n"); }
#define ALLOCATION_ERROR(); { fprintf(stderr, "Memory allocation error. Please quit and retry"); }
#define MAX_ALPHA 26

typedef struct Trie {
	bool terminal;
	struct Trie* child[MAX_ALPHA];
} Trie;


typedef struct OpNode {
	int value;
	int decimal;
	char code[20];
	struct OpNode* next;
} OpNode;


Trie* root;
OpNode* hashTable[MAX_HASH_SIZE];
OpNode* hashTailPointer[MAX_HASH_SIZE];

unsigned char vMemory[MAX_MEMORY_LINE * MAX_BYTES_LINE];

extern bool quit_flag;

extern void init();
extern void makeHashTable();



extern Trie* getNewTrieNode();
extern void deleteTrie(Trie* root);
extern void insertTrie(Trie* root, char* key);
extern bool searchTrie(Trie* root, char* key);
#endif
