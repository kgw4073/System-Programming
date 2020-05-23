#ifndef _COMMAND_H
#define _COMMAND_H
#include "20150514.h"

#define MAX_ALPHA 26
#define MAX_PARSED_NUM 10
#define MAX_DUMP_BYTE 16
#define MAX_DUMP_LINE 10
#define WRONG_HEXA -1

#define STDERR_ADDRESS_ERROR(); { fprintf(stderr, "정상적인 주소를 입력하세요. (0x00000 ~ 0xFFFFF)\n");}
#define STDERR_MEMORY_CORRUPT(); { fprintf(stderr, "Memory Corruption! 정상적인 범위를 입력하세요.\n"); }
#define STDERR_VALUE_ERROR(); {	fprintf(stderr, "Value error! 정상적인 값을 입력하세요 (0x00 ~ 0xff)\n"); }
#define STDERR_COMMAND_ERROR(); { fprintf(stderr, "Command error! 정상적인 명령어를 입력하세요. 도움이 필요하면 h[elp]를 입력하세요. \n"); }
#define STDERR_OPCODE_ERROR(); { fprintf(stderr, "Operation Code error! 정상적인 OPCODE를 입력하세요. 도움이 필요하면 opcodelist를 입력하세요. \n"); }
#define STDERR_OPERAND_ERROR(); { fprintf(stderr, "Operand Code error! 정상적인 Operand를 입력하세요 \n"); }
#define ASSEMBLE_FILE_ERROR(); { fprintf(stderr, "해당 파일은 .asm형식이 아닙니다. \n"); }
#define DUPLICATE_SYMBOL_ERROR(); { fprintf(stderr, "SYMBOL이 중복되었습니다. \n"); }
#define OTHER_ERROR(); { fprintf(stderr, "에러가 발생했습니다. \n"); }

typedef enum {
	NORMAL, ADDRESS_INPUT_ERROR, MEMORY_INDEX_ERROR, VALUE_ERROR, COMMAND_ERROR, OPCODE_ERROR
	, FILE_OPEN_ERROR, ASSEMBLE_FILE_ERROR, DUPLICATE_SYMBOL_ERROR, OPERAND_ERROR, BASE_NO_EXIST_ERROR,
	TOO_FAR_ERROR, OPERAND_NOT_FOUND_ERROR
} RETURN_CODE;

/*
	사전에 등록되어 있는 명령어들을 26진수로 보고 라빈-카프 해시 값을 변형하여 미리 계산한 값이다.
*/
enum input_command {
	h = 8, help = 289466, d = 4, dir = 12406, q = 17, quit = 358167, hi = 242,
	history = -645956178, 
	du = 550, dump = 290554,
	e = 5, edit = 357713, f = 6, fill = 219264, reset = 9240392, 
	opcode = 61500883, opcodelist = -1674749357, 
	type = 99366, assemble = 946171891, symbol = 149475761,
	progaddr = -172525656, loader = 216221030, bp = 418, run = 10028
};

/*
	history를 담을 linked list
*/
typedef struct historyNode {
	char* command;
	struct historyNode* next;
} historyNode;

/*
	SYMBOL TABLE을 Binary Search Tree로 구현
*/
typedef struct Symbol {
	char symbol[20];
	int loc;
	struct Symbol* left, * right;
} Symbol;

int parameters[3];
int lastAddress;
int current_command;


historyNode* headOfHistory, * tailOfHistory;

/*
	Project 1에서 추가한 함수
*/
extern void commandParse(char instruction[MAX_COMMAND_SIZE]);
extern void insertHistory(char instruction[MAX_COMMAND_SIZE]);
extern void deleteHistory();
extern void showHistory();
extern void showHelp();
extern void showDir();
extern void resetMemory();
extern void showOpcodelist();

extern void playCommand(char parsedInstruction[][MAX_PARSED_NUM + 10], enum input_command current_command);
extern void dumpMemory(int parameters[]);
extern void editMemory(int parameters[]);
extern void fillMemory(int parameters[]);
extern void showMnemonic(int parameters[]);
extern RETURN_CODE isExecutable(enum input_command current_command, char parsedInstruction[][MAX_PARSED_NUM + 10], int* parsedReference);
extern bool isOverflowed(int address);
extern int toDecimal(char c);

/*
	Project 2에서 추가한 함수
*/
extern void showTreeSymbol(Symbol* root);
extern void typeFile(char parsedInstruction[][MAX_PARSED_NUM + 10]);




#endif