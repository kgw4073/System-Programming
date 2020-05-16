/*-------------------------------------------------------------- *
 *                                                               *
 * Subject : System Programming									 *	
 * Project 1 : SIC/XE Machine Shell								 *
 * Project 2 : SIC/XE Assembler								     *  
 *																 *
 * Student ID: 20150514                                          *
 *																 *
 * File name: 20150514.c                                         *
 * File description: Main file for the project.					 *	
 *																 *
 * Made by Keonwoo Kim											 *
 *---------------------------------------------------------------*/

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
#include "linkloader.h"
#define CTOI(x) ((int)x-(int)'a')

bool quit_flag = false;
Trie* root = NULL;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/*																				  //
 *								Trie에 관한 함수들								  //
 *																				  //
 * getNewTrieNode(), insertTrie, searchTrie()									  */
// 새로운 트라이 노드를 하나 생성하는 함수.
Trie* getNewTrieNode() {
	Trie* node = NULL;
	node = (Trie*)malloc(sizeof(Trie));
	if (node) {
		node->terminal = false;
		node->number = 0;
		for (int i = 0; i < MAX_ALPHA; i++) {
			node->child[i] = NULL;
		}
	}
	return node;
}

// 트라이에 key 문자열을 삽입함.
void insertTrie(Trie* root, char* key) {
	size_t length = strlen(key);
	Trie* temp = root;
	int num = 0, base = 1;
	for (size_t i = 0; i < length; i++) {
		int next = CTOI(key[i]);
		if (!temp->child[next]) {
			temp->child[next] = getNewTrieNode();
		}
		temp = temp->child[next];
		num += (next + 1) * base;
		base *= 26;
	}
	temp->number = num;
	temp->terminal = true;
}

// key에 해당하는 문자열이 있는지 판단.
int searchTrie(Trie* root, char* key) {
	size_t length = strlen(key);
	Trie* temp = root;

	for (size_t i = 0; i < length; i++) {
		int next = CTOI(key[i]);
		if (next < 0 || next > 25) {
			return 0;
		}

		if (!temp->child[next]) {
			return false;
		}
		temp = temp->child[next];
	}
	if (temp != NULL && temp->terminal) return temp->number;
	else return 0;
}

// 프로그램이 종료될 때 동적 할당된 트라이 노드들을 전부 해제.
void deleteTrie(struct Trie* root) {
	if (root) {
		for (int i = 0; i < MAX_ALPHA; i++) {
			if (root->child[i]) {
				deleteTrie(root->child[i]);
			}
		}
		free(root);
	}
}
void makeTrie() {
	char basicInstruction[24][100] = {
"h", "help", "d", "dir", "q", "quit", "hi", "history", "du", "dump",
"e", "edit", "f", "fill", "reset", "opcode", "opcodelist", "type", "assemble", "symbol",
"progaddr", "loader", "bp", "run"
	};
	root = getNewTrieNode();
	for (int i = 0; i < 24; i++) {
		insertTrie(root, basicInstruction[i]);
	}

}
//   						End of Trie Function								  //
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


/*--------------------------------------------------------------------------------------------------*
 *	Hash Table에 관한 함수들.																			*
 *  getNewHashNode(), insertHashEntry(), makeHashTable(), deleteHashTable()							*
 *																									*
 *--------------------------------------------------------------------------------------------------*/
OpNode* getNewHashNode() {
	OpNode* temp = (OpNode*)malloc(sizeof(OpNode));
	// 동적 할당 안되면 오류
	if (temp == NULL) {
		ALLOCATION_ERROR();
		return NULL;
	}
	// 전부 0으로 초기화.
	temp->value = 0;
	temp->decimal = 0;
	temp->format = 0;
	memset(temp->code, 0, sizeof(temp->code));
	temp->next = NULL;
	return temp;
}

// Hash Table에 Node를 추가.
// OpCodeDecimal : 해당 OpCode를 26진수로 보고 이를 10진수로 바꾼 수
// value : opcode.txt에 들어가있는, OpCode에 대응되는 값.
// code[] : opcode 문자열.
void insertHashEntry(unsigned long OpCodeDecimal, int value, char code[], char form[]) {
	OpNode* temp = getNewHashNode();
	if (temp == NULL) return;
	// opcode를 20으로 나눈 나머지가 hash table의 entry index가 된다.
	unsigned long entryIndex = OpCodeDecimal % MAX_HASH_SIZE;
	hashTailPointer[entryIndex]->value = value;
	hashTailPointer[entryIndex]->decimal = OpCodeDecimal;
	hashTailPointer[entryIndex]->format = form[0] - '0';

	strcpy(hashTailPointer[entryIndex]->code, code);
	hashTailPointer[entryIndex]->next = temp;
	hashTailPointer[entryIndex] = temp;
}

// shell이 끝날 때 Hash Table의 동적 할당 해제.
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

// Hash Table을 만듦.
void makeHashTable() {
	for (int i = 0; i < MAX_HASH_SIZE; i++) {
		hashTable[i] = getNewHashNode();
		// 동적 할당 안되면 강제 종료.
		if (hashTable[i] == NULL) {
			deleteHashTable();
			exit(-1);
		}
		// hashTailPointer는 hash Entry에 해당하는 각각의 큐의 끝을 가리켜서, 출력 혹은 삽입할 때 사용됨.
		hashTailPointer[i] = hashTable[i];
	}
	FILE* fp = fopen("opcode.txt", "r");
	if (fp == NULL) {
		FILE_ERROR();
		exit(-1);
	}
	int res = 0;
	int value;
	char code[20], form[20];
	// 주어진 양식대로 읽어들임. EOF가 나올 때까지.
	while ((res = fscanf(fp, "%02X %s %s", &value, code, form)) != EOF) {
		int len = (int)strlen(code);
		unsigned long OpCodeDecimal = 0;
		unsigned long base = 1;
		// OpCode를 26진수로 보고 OpCodeDecimal을 계산하여 Hash Table에 삽입함.
		for (int i = 0; i < len; i++) {
			OpCodeDecimal += base * (code[i] - 'A' + 1);
			base *= 26;
		}
		insertHashEntry(OpCodeDecimal, value, code, form);
	}
	fclose(fp);
}

OpNode* searchHashNode(char* Opcode, int length) {
	unsigned long entry = 0;
	unsigned long base = 1;
	for (int i = 0; i < length; i++) {
		entry += ((Opcode[i] - 'A' + 1) * base);
		base *= 26;
	}
	
	OpNode* search = hashTable[entry % MAX_HASH_SIZE];
	if (search->next == NULL) return NULL;
	//search = search->next;
	while (search != NULL) {
		if (search->decimal == entry) return search;
		search = search->next;
	}
	return NULL;
}


// 기본적인 명령어들을 바탕으로 트라이를 생성하고 history를 담기 위한 queue를 생성
// headOfHistory와 tailOfHistory는 History queue에서 각각 head와 tail을 의미.
void init() {
	Progaddr = 0;
	memset(BPlist, -1, sizeof(BPlist));

	makeTrie();
	headOfHistory = (historyNode*)malloc(sizeof(historyNode));
	tailOfHistory = headOfHistory;

	// debuging을 위해 임의로 메모리 초기화
	unsigned char temp[MAX_MEMORY_LINE * MAX_BYTES_LINE];
	for (int i = 0; i < MAX_MEMORY_LINE * MAX_BYTES_LINE; i++) {
		temp[i] = (unsigned char)(rand() % 256);
	}
	memcpy(vMemory, temp, sizeof(vMemory));
	makeHashTable();
}


int main() {
	// Trie와 Hash Table 만듦.
	init();

	// quit_flag가 세팅될 때까지 계속해서 쉘을 실행.
	while (1) {
		if (quit_flag) break;
		printf("sicsim> ");

		char instruction[MAX_COMMAND_SIZE];
		memset(instruction, 0, sizeof(instruction));

		// 한 줄을 통째로 instruction 배열에 받음.
		fgets(instruction, MAX_COMMAND_SIZE, stdin);
		int len = (int)strlen(instruction);
		// fgets로 받으면 마지막에 \n이 삽입되므로 이 값을 \0으로 바꿔줨.
		instruction[len - 1] = '\0';

		// 들어온 instruction을 Parsing함.
		commandParse(instruction);
	}

	// quit되면 동적 할당된 메모리들 전부 해제.
	deleteHashTable();
	deleteHistory();
	deleteTrie(root);
	return 0;
}
