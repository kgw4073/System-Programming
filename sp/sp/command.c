#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "command.h"
#include "20150514.h"

void insert_queue(char instruction[MAX_COMMAND_SIZE]) {
	if (head_of_command_queue == tail_of_command_queue) {
		head_of_command_queue = (struct command_list*)malloc(sizeof(struct command_list));
		int len = strlen(instruction);
		head_of_command_queue->command = (char*)malloc(len*sizeof(char));
		head_of_command_queue->next = NULL;
		strncpy(head_of_command_queue, instruction, len);
		tail_of_command_queue = head_of_command_queue->next;

	}
	else {
		tail_of_command_queue = (struct command_list*)malloc(sizeof(struct command_list));
		int len = strlen(instruction);
		tail_of_command_queue->command = (char*)malloc(len * sizeof(char));
		tail_of_command_queue->next = NULL;
		strncpy(tail_of_command_queue, instruction, len);
		tail_of_command_queue = tail_of_command_queue->next;
	}
}

void delete_queue() {
	command_list* temp = head_of_command_queue;
	if (temp->next) {
		temp = temp->next;
		free(head_of_command_queue->command);
		free(head_of_command_queue);
		head_of_command_queue = temp;
	}
	if (temp) {
		free(temp->command);
		free(temp);
	}
}

void print_queue() {
	int count = 1;
	command_list* temp = head_of_command_queue;
	while (temp != tail_of_command_queue) {
		printf("%-4d ", count++);
		printf("%s\n", temp->command);
	}
}


void show_help() {
	printf("h[elp]\nd[ir]\n\q[uit]\nhi[story]\n\
du[mp][start, end]\ne[dit] address, value\nf[ill] start, end, value\n\
reset\nopcode mnemonic\nopcodelist\n");

}

void show_dir() {
	DIR* dp = NULL;
	struct dirent* entry;
	struct stat buf;

	if ((dp = opendir(".")) == NULL) {
		printf("[Error] cannot open ..\n");
		return;
	}
	int counter = 0;
	while ((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &buf);
		printf("   ");
		if (S_ISDIR(buf.st_mode)) {
			printf("%20s/", entry->d_name);
		}
		else if(S_IXUSR & buf.st_mode) {
			printf("%20s*", entry->d_name);
		}
		else {
			printf("%20s", entry->d_name);
		}
		if (++counter % 4 == 0) {
			printf("\n");
		}
	}
	if (counter % 4) printf("\n");
	closedir(dp);
	return;
}

bool operand_parsing(char insturction[MAX_COMMAND_SIZE], int command_number, int command_length) {
	bool operand_parse_flag = true;
	switch (command_number) {

	case dump:
		break;

	case edit:
		break;

	case fill:
		break;


	default:
		operand_parse_flag = false;

	}
	return operand_parse_flag;

}

void reset_memory() {
	;
}

bool command_parsing(char instruction[MAX_COMMAND_SIZE]) {
	char temp[MAX_COMMAND_SIZE];
	
	int parsed_length = 0;
	bool history_flag = false;
	bool success_flag = true;

	bool blank_buffer = false;
	for (int i = 0; i < MAX_COMMAND_SIZE; i++) {
		if (instruction[i] == ' ') {
			if (!blank_buffer && parsed_length > 0) {
				blank_buffer = true;
				temp[parsed_length++] = instruction[i];
			}
		}
		else if (instruction[i] == '\n') {
			temp[parsed_length] = '\0';
			break;
		}

		else {
			blank_buffer = true;
			if (blank_buffer) {
				temp[parsed_length++] = instruction[i];
			}
			blank_buffer = false;
		}
	}
	memcpy(instruction, temp, sizeof(temp));


	if (!strcmp(instruction, "h") || !strcmp(instruction, "help")) {

		show_help();

	}
	else if (!strcmp(instruction, "d") || !strcmp(instruction, "dir")) {
		show_dir();
	}
	else if (!strcmp(instruction, "q") || !strcmp(instruction, "quit")) {
		quit_flag = true;
	}
	else if (!strcmp(instruction, "hi") || !strcmp(instruction, "history")) {
		history_flag = true;
		insert_queue(instruction);
		print_queue();
	}
	else if (!strncmp(instruction, "du", 2) || !strncmp(instruction, "dump", 4)) {
		if (operand_parsing(instruction, dump, 2) || operand_parsing(instruction, dump, 4)) {

		}
		else {
			success_flag = false;
		}
	}
	else if (!strncmp(instruction, "e", 1) || !strncmp(instruction, "edit", 4)) {
		if (operand_parsing(instruction, edit, 1) || operand_parsing(instruction, edit, 4)) {

		}
		else {
			success_flag = false;
		}
	}

	else if (!strncmp(instruction, "f", 1) || !strncmp(instruction, "fill", 4)) {
		if (operand_parsing(instruction, fill, 1) || operand_parsing(instruction, fill, 4)) {

		}
		else {
			success_flag = false;
		}
	}
	else if (!strcmp(instruction, "reset")) {
		reset_memory();
	}
	else if (!strncmp(instruction, "opcode", 6)) {
		if (operand_parsing(instruction, opcode, 6)) {

		}
	}
	else if (!strcmp(instruction, "opcodelist")){

	}
	else {
		success_flag = false;

	}
	if (success_flag) {
		insert_queue(instruction);
	}
	return success_flag;

}
