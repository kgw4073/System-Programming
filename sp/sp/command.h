#ifndef _COMMAND_H
#define _COMMAND_H
#include "20150514.h"
#define MAX_ALPHA 26

typedef struct command_list {

	char* command;
	struct command_list* next;

} command_list;





enum input_command {
	help, dir, quit, history, dump, edit,
	fill, reset, opcode, opcodelist
};

command_list* head_of_command_queue, *tail_of_command_queue;

extern void insert_queue(char instruction[MAX_COMMAND_SIZE]);
extern void delete_queue();
extern void print_queue();
extern void show_help();
extern void show_dir();
extern void reset_memory();

extern bool operand_parsing(char insturction[MAX_COMMAND_SIZE], int command_number, int command_length);
extern bool command_parsing(char instruction[MAX_COMMAND_SIZE]);

#endif