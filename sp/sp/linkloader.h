#ifndef _LINK_LOADER_H_
#define _LINK_LOADER_H_
#define RELOC_ADDR CSAddr
int Progaddr;
int BPlist[1000];
int sizeOfBP;
int Relocations[1000];

//typedef struct ExtSymbol {
//	char Symbol[100];
//	int addr;
//	struct ExtSymbol* next;
//} ExtSymbol;

typedef struct ESTab {
	char ControlSection[100];
	int CSAddr, length;
	struct ESTab* left, * right;
} ESTab;

extern ESTab* ESTabRoot;

extern void link_and_load(char parsedInstruction[][MAX_PARSED_NUM + 10]);
extern void BreakPoint(int parameters[], char str[]);
extern void setProgaddr(int parameters[]);

#endif