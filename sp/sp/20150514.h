
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



//struct dumped {
//	int last_address;
//};
typedef struct OpNode {
	int value;
	int decimal;
	char code[20];
	struct OpNode* next;
} OpNode;

OpNode* hashTable[MAX_HASH_SIZE];
OpNode* hashTailPointer[MAX_HASH_SIZE];
unsigned char vMemory[MAX_MEMORY_LINE * MAX_BYTES_LINE];

extern bool quit_flag;

extern void init();
extern void makeHashTable();

#endif
