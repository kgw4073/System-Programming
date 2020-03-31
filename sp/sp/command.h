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
extern int parameters[3];

typedef enum {
	NORMAL, ADDRESS_INPUT_ERROR, MEMORY_INDEX_ERROR, VALUE_ERROR, COMMAND_ERROR, OPCODE_ERROR
} RETURN_CODE;

typedef struct command_list {
	char* command;
	struct command_list* next;

} command_list;


enum input_command {
	help, dir, quit, history, dump, edit,
	fill, reset, opcode, opcodelist
};

extern int lastAddress;
int current_command;
command_list* head_of_command_queue, *tail_of_command_queue;

extern void commandParse(char instruction[MAX_COMMAND_SIZE]);
extern void insertHistory(char instruction[MAX_COMMAND_SIZE]);
extern void deleteHistory();
extern void showHistory();
extern void showHelp();
extern void showDir();
extern void resetMemory();
extern void showOpcodelist();

extern void playCommand(int current_command, char parsedInstruction[][MAX_PARSED_NUM + 10], int parsedNumber);
extern void dumpMemory(int parameters[], int parsedNumber);
extern void editMemory(int parameters[]);
extern void fillMemory(int parameters[]);
extern void showMnemonic(int parameters[]);

extern RETURN_CODE isExecutable(enum input_command current_command, char parsedInstruction[][MAX_PARSED_NUM + 10], int* parsedReference);
extern bool isOverflowed(int address);
extern void lstat(const char*, struct stat*);
extern int toDecimal(char c);

#endif