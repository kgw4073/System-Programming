#ifndef _COMMAND_H
#define _COMMAND_H
#include "20150514.h"
#define MAX_ALPHA 26
#define MAX_PARSED_NUM 10
#define MAX_DUMP_BYTE 16
#define MAX_DUMP_LINE 10
#define WRONG_HEXA -1
inline int hexa(char c) {
	if ('0' <= c && c <= '9') return c - '0';
	else if ('a' <= c && c <= 'z') return c - 'a' + 10;
	else if ('A' <= c && c <= 'Z') return c - 'A' + 10;
	else return -1;
}
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


extern void insert_queue(char instruction[MAX_COMMAND_SIZE]);
extern void delete_queue();
extern void print_queue();
extern void show_help();
extern void show_dir();
extern void reset_memory();
extern void playCommand(int current_command, char parsedInstruction[][MAX_PARSED_NUM + 10]);


extern bool operand_parsing(char insturction[MAX_COMMAND_SIZE], int command_number, int command_length);
extern bool command_parsing(char instruction[MAX_COMMAND_SIZE]);

#endif