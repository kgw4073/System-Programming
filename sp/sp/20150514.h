
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

// Trie 구조체
typedef struct Trie {
	bool terminal;
	struct Trie* child[MAX_ALPHA];
} Trie;

// Opcode에 대한 정보를 담음. opcode Hash Table을 만들기 위함.
typedef struct OpNode {
	// value는 실제 부여된 값.
	int value;
	// decimal은 opcode를 26진수로 보았을 때 가리키는 10진수 값.
	int decimal;
	// 실제 opcode 문자열
	char code[20];
	// hash table에서 연결 리스트를 구현하기 위함.
	struct OpNode* next;
} OpNode;

// 가상 메모리를 2^20 * sizeof(unsigned char) = 1MB 짜리 선언
unsigned char vMemory[MAX_MEMORY_LINE * MAX_BYTES_LINE];


// Trie* root는 명령어를 저장하는 문자열 트라이
Trie* root;

// hashTable은 opcode 를 저장하기 위한 hashTable. 연결 리스트로 구현
OpNode* hashTable[MAX_HASH_SIZE];

// hashTailPointer는 hashTable을 출력하거나 hashTable에 insert할 때 끝에 바로 추가하기 위해 각 entry에 대해 가장 끝 리스트를 가리킴.
OpNode* hashTailPointer[MAX_HASH_SIZE];

// quit_flag는 우리가 명령어를 quit으로 쳤을 때 setting됨. command.c에서도 쓰기 위해 20150514.h에 선언.
bool quit_flag;


extern void init();
extern void makeTrie();
extern void makeHashTable();


extern OpNode* getNewHashNode();
extern void insertHashEntry(int OpCodeDecimal, int value, char code[]);
extern Trie* getNewTrieNode();
extern void deleteTrie(Trie* root);
extern void insertTrie(Trie* root, char* key);
extern bool searchTrie(Trie* root, char* key);
#endif
