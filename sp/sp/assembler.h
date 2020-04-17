#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_
#include "20150514.h"
#include "command.h"

typedef enum REG {
	A, X, L, B, S, T, F, PC = 8, SW = 9
} REG;

typedef struct Symbol {
	char symbol[20];
	int loc;
	struct Symbol* left, * right;
} Symbol;

Symbol* symbolRoot;
int ProgramCounter, BaseAddress;
char BaseSymbol[30];
typedef struct Formats {
	//unsigned int op : 8;
	//unsigned int r1 : 4;
	//unsigned int r2 : 4;
	//unsigned int n : 1;
	//unsigned int i : 1;
	//unsigned int x : 1;
	//unsigned int b : 1;
	//unsigned int p : 1;
	//unsigned int e : 1;
	//int disp : 12;
	//unsigned int address : 20;
	unsigned int code;
	int format;
	int loc;
	char label[30], opcode[30], operand_first[30], operand_second[30];
	RETURN_CODE ret;
} Formats;

Formats objectCode[100000];

char lst[30];
char obj[30];


extern bool pass1(char filename[MAX_PARSED_NUM + 10]);
extern bool pass2(char filename[MAX_PARSED_NUM + 10]);
extern void doAssemble(char parsedInstruction[][MAX_PARSED_NUM + 10]);


#endif