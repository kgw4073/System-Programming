#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_
#include "20150514.h"
#include "command.h"

typedef enum REG {
	A, X, L, B, S, T, F, PC = 8, SW = 9
} REG;

Symbol* symbolRoot;

extern int ProgramCounter, BaseAddress;

char BaseSymbol[30];

typedef struct Formats {

	unsigned int op : 8;
	unsigned int reg1 : 4;
	unsigned int reg2 : 4;
	unsigned int nixbpe : 6;
	unsigned int disp : 12;
	unsigned int addr : 20;
	bool isComment;
	bool isConstant;
	bool isVariable;
	bool isDirective;
	unsigned obj;
	int format;
	int loc;
	int parts;
	char label[30], opcode[30], operand_first[30], operand_second[30];
	char* comment;
	RETURN_CODE ret;
} Formats;

typedef struct Modifications {
	int loc;
	struct Modifications* next;
} Modifications;

Modifications* HeadModifyRecord, *TailModifyRecord;

Formats objectCode[1000];

char lstFile[30];
char objFile[30];
int assemble_start_line;
int assemble_end_line;

extern void reverseString(char str[]);
extern REG findRegNumber(char str[]);

extern void objectCodeWrite(int line, int loc, int parts, bool isFormat4, OpNode* search, int constant, char part1[], char part2[], char part3[], char part4[], bool labelFlag);

extern bool insertBinaryTreeSymbol(Symbol* root, char symbol[], int loc);
extern Symbol* findBinaryTreeSymbol(Symbol* root, char symbol[]);
extern Symbol* getNewSymbolNode();
extern void deleteSymbolTree(Symbol* root);

extern void writeFiles(char lstFile[], char objFile[]);
extern void showPassError();
extern bool pass1(char filename[]);
extern bool pass2();
extern void doAssemble(char parsedInstruction[][MAX_PARSED_NUM + 10]);

#endif