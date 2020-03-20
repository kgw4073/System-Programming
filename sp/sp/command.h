#ifndef _COMMAND_H
#define _COMMAND_H
#include "20150514.h"
#define MAX_ALPHA 26
#define MAX_PARSED_NUM 10
#define MAX_DUMP_BYTE 16
#define MAX_DUMP_LINE 10
#define WRONG_HEXA -1
#define STDERR_MEMORY_CORRUPT(); { fprintf(stderr, "Memory corrupted! 정상적인 범위를 입력하세요 (0x00000~0xfffff)\n");}
#define STDERR_VALUE_ERROR(); {	fprintf(stderr, "Value error! 정상적인 값을 입력하세요 (0x00 ~ 0xff)\n"); }
#define STDERR_COMMAND_ERROR(); { fprintf(stderr, "Command error! 정상적인 명령어를 입력하세요. h[elp] \n"); }
static int parameters[3] = { 0, };

typedef struct command_list {

	char* command;
	struct command_list* next;

} command_list;


enum input_command {
	help, dir, quit, history, dump, edit,
	fill, reset, opcode, opcodelist
};

static int lastAddress = 0;
int current_command;
command_list* head_of_command_queue, *tail_of_command_queue;


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

extern bool command_parsing(char instruction[MAX_COMMAND_SIZE]);

extern inline bool isOverflowed(int address);
extern inline int toDecimal(char c);
extern bool isExecutable(enum input_command current_command, char parsedInstruction[][MAX_PARSED_NUM + 10], int* parsedNum);

#endif