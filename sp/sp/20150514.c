#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include "20150514.h"
#include "command.h"
#include "BasicCommand.h"

bool quit_flag = false;
Trie* root = NULL;

void init() {
	char basicInstruction[17][100] = {
	"h", "help", "d", "dir", "q", "quit", "hi", "history", "du", "dump",
	"e", "edit", "f", "fill", "reset", "opcode", "opcodelist"
	};
	root = getNewTrieNode();
	for (int i = 0; i < 17; i++) {
		insertTrie(root, basicInstruction[i]);
	}
	unsigned char temp[MAX_MEMORY_LINE * MAX_BYTES_LINE];
	for (int i = 0; i < MAX_MEMORY_LINE * MAX_BYTES_LINE; i++) {
		temp[i] = (unsigned char)(rand() % 256);
	}
	memcpy(vMemory, temp, sizeof(vMemory));
	head_of_command_queue = (command_list*)malloc(sizeof(command_list));
	tail_of_command_queue = head_of_command_queue;
}
OpNode* getNewHashNode() {
	OpNode* temp = (OpNode*)malloc(sizeof(OpNode));
	if (temp == NULL) {
		ALLOCATION_ERROR();
		return NULL;
	}
	temp->value = 0;
	temp->decimal = 0;
	memset(temp->code, 0, sizeof(temp->code));
	temp->next = NULL;
	return temp;
}

void insertHashEntry(int OpCodeDecimal, int value, char code[]) {
	OpNode* temp = getNewHashNode();
	if (temp == NULL) return;
	int entryIndex = OpCodeDecimal % MAX_HASH_SIZE;
	hashTailPointer[entryIndex]->value = value;
	hashTailPointer[entryIndex]->decimal = OpCodeDecimal;
	strcpy(hashTailPointer[entryIndex]->code, code);
	hashTailPointer[entryIndex]->next = temp;
	hashTailPointer[entryIndex] = temp;
}
void deleteHashTable() {
	for (int i = 0; i < MAX_HASH_SIZE; i++) {
		OpNode* head = hashTable[i];

		while (1) {
			if (!head) break;
			OpNode* temp = head->next;
			free(head);
			head = temp;
		}
	}
}

void makeHashTable() {
	for (int i = 0; i < MAX_HASH_SIZE; i++) {
		hashTable[i] = getNewHashNode();
		if (hashTable[i] == NULL) {
			deleteHashTable();
			exit(-1);
		}
		hashTailPointer[i] = hashTable[i];
	}
	FILE* fp = fopen("./opcode.txt", "r");
	if (fp == NULL) {
		FILE_ERROR();
		exit(-1);
	}
	int res = 0;
	int value;
	char code[20], cycle[20];
	while ((res = fscanf(fp, "%02X %s %s", &value, code, cycle)) != EOF) {
		int len = (int)strlen(code);
		int OpCodeDecimal = 0;
		int base = 1;
		for (int i = 0; i < len; i++) {
			OpCodeDecimal += base * (code[i] - 'A');
			base *= 26;
		}
		insertHashEntry(OpCodeDecimal, value, code);
	}
	close(fp);
}



int main() {


	init();
	makeHashTable();
	while (1) {
		if (quit_flag) break;
		printf("sicsim> ");

		char instruction[MAX_COMMAND_SIZE];
		memset(instruction, 0, sizeof(instruction));
		fgets(instruction, MAX_COMMAND_SIZE, stdin);
		int len = (int)strlen(instruction);
		instruction[len - 1] = '\0';
		commandParse(instruction);
	}

	deleteHashTable();
	deleteHistory();
	deleteTrie(root);
	return 0;
}