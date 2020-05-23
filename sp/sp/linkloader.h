#ifndef _LINK_LOADER_H_
#define _LINK_LOADER_H_
#define RELOC_ADDR CSAddr
#define PRINT_BP_FLAG -2
#define CLEAR_BP_FLAG -1

// Progaddr은 메모리 로딩 시작 주소.
int Progaddr;
// Breakpoint list의 길이
int sizeOfBP;

// START_EXECADDR과 END_EXECADDR은 Pass1, Pass2를 거쳐 세팅됨.
int START_EXECADDR, END_EXECADDR;

// Breakpoint를 담는 리스트.
int BPlist[1000];

// 각 Control Section마다 사용될 Relocation table
int Relocations[1000];

// Register 배열
int Register[10];

// External Symbol
typedef struct ExtSymbol {
	char Symbol[100];
	int addr;
	struct ExtSymbol* next;
} ExtSymbol;

// LoadMap : Control Section에 대한 정보를 담는 BST
// 한 Control Section에는 ExtSymbol Queue가 필요하므로 *front와 *rear가 존재함.
typedef struct LoadMap {
	char ControlSection[100];
	int CSAddr, length;
	struct ExtSymbol* front, * rear;
	struct LoadMap* left, * right;
} LoadMap;
extern LoadMap* LoadmapRoot;

// Control Section과 Symbol에 대한 정보를 전부 저장하는 BST
typedef struct ESTab {
	char ControlSection[100];
	int CSAddr, length;
	struct ESTab* left, * right;
} ESTab;
extern ESTab* ESTabRoot;

// Link를 위해 올려지는 Program의 길이를 저장.
extern int ProgramLengthForLink;

// OPCode를 enum type으로 저장.
typedef enum OPCode {
	ADD = 0x18, ADDF = 0x58, ADDR = 0x90, AND = 0x40, CLEAR = 0xB4,
	COMP = 0x28, COMPF = 0x88, COMPR = 0xA0, DIV = 0x24,
	DIVF = 0x64, DIVR = 0x9C, FIX = 0xC4, FLOAT = 0xC0, 
	HIO = 0xF4, J = 0x3C, JEQ = 0x30, JGT = 0x34, JLT = 0x38, 
	JSUB = 0x48, LDA = 0x00, LDB = 0x68, LDCH = 0x50, LDF = 0x70, 
	LDL = 0x08, LDS = 0x6C, LDT = 0x74, LDX = 0x04, LPS = 0xD0,
	MUL = 0x20, MULF = 0x60, MULR = 0x98, NORM = 0xC8, OR = 0x44,
	RD = 0xD8, RMO = 0xAC, RSUB = 0x4C, SHIFTL = 0xA4, SIO = 0xF0,
	SSK = 0xEC, STA = 0x0C, STB = 0x78, STCH = 0x54, STF = 0x80, 
	STI = 0xD4, STL = 0x14, STS = 0x7C, STSW = 0xE8, STT = 0x84, 
	STX = 0x10, SUB = 0x1C, SUBF = 0x5C, SUBR = 0x94, SVC = 0xB0, 
	TD = 0xE0, TIO = 0xF8, TIX = 0x2C, TIXR = 0xB8, WD = 0xDC
} OPCode;



extern void deleteSymbolQ(ExtSymbol* root);
extern void deleteLoadmap(LoadMap* root);
extern void deleteESTabRoot(ESTab* root);
extern ESTab* findESTab(ESTab* root, char ControlSection[]);
extern ExtSymbol* getNewExtSymbolNode();
extern ESTab* getNewESTabNode();
extern LoadMap* getNewLoadMapNode();
extern LoadMap* insertLoadMap(LoadMap* root, char ControlSection[], int CSAddr, int length);
extern void insertESTab(ESTab* root, char ControlSection[], int CSAddr, int length);
extern void insertLoadMapQ(LoadMap* root, char name[], int nodeAddr);
extern void PrintPass1Info_LoadMap(int ProgramTotalLength);
extern void PrintCurrentLoadMapQ(LoadMap* root);
extern void PrintLoadMap(LoadMap* root);

extern bool LinkingPass1(char filenames[][MAX_PARSED_NUM + 10]);
extern bool LinkingPass2(char filenames[][MAX_PARSED_NUM + 10]);
extern bool isInBreakPointList(int current_addr);
extern int WhatIsFormat(int op);
extern void Execute(int format, int* ProgramCounter, int op, int nixbpe, int disp, int sign_flag, int r1, int r2);
extern void PrintCurrentRegisters();
extern void Run();
extern void link_and_load(char parsedInstruction[][MAX_PARSED_NUM + 10]);
extern void BreakPoint(int parameters[], char str[]);
extern void setProgaddr(int parameters[]);

#endif