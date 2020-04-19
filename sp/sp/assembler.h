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

typedef struct Modification {
	int starting;
	int loc;
	int length;
} Modifications;

Modifications* ModifyRecord;


Formats objectCode[100000];

char lst[30];
char obj[30];
int assemble_end_line;

extern bool pass1(char filename[MAX_PARSED_NUM + 10]);
extern bool pass2(char filename[MAX_PARSED_NUM + 10]);
extern void doAssemble(char parsedInstruction[][MAX_PARSED_NUM + 10]);


#endif