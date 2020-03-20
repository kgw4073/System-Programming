#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include "command.h"
#include "20150514.h"
#include "BasicCommand.h"

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


void playCommand(int current_command, char parsedInstruction[][MAX_PARSED_NUM + 10]) {
	switch (current_command) {
	case help:

		break;

	case dir:
		break;

	case quit:
		break;

	case history:
		break;

	case dump:
		break;

	case edit:
		break;

	case fill:
		break;

	case reset:
		break;

	case opcode:
		break;

	case opcodelist:
		break;

	}
}

bool isExecutable(int current_command, char parsedInstruction[][MAX_PARSED_NUM + 10]) {
	bool ret = true;
	int parsedNumber = 0;
	for (int i = 0; ; i++) {
		if (!parsedInstruction[i]) break;
		parsedNumber++;
	}
	switch (current_command) {
	case dump:
		int startAddress = 0;
		int hexaDecimal = 0;
		int lastIndex = strlen(parsedInstruction[1]) - 1;
		if (parsedNumber == 1) {
			startAddress = lastAddress;
			lastAddress += MAX_DUMP_BYTE * MAX_DUMP_LINE;

		}
		else if (parsedNumber == 2) {
			if (parsedInstruction[1][lastIndex] == ',') {
				for (int i = lastIndex - 1; i >= 0; i--) {
					if ((hexaDecimal = hexa(parsedInstruction[1][i])) == WRONG_HEXA) {
						ret = false;
						break;
					}
					else {
						startAddress+=
					}
				}
			}
			else {
				ret = false;
			}
		}
		else if (parsedNumber == 3) {
			if (parsedInstruction[1][lastIndex] == ',') {
				parsedInstruction[1][lastIndex] = '\0';
				

			}
			else {
				ret = false;
			}
		}
		else {
			ret = false;
		}
		break;

	case edit:
		if (parsedNumber == 3) {
			if()
		}
		else ret = false;
		break;

	case fill:

		break;

	case opcode:

		break;

	default:
		if (parsedNumber == 1) {
			ret = false;
		}
		break;
	}
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

	int len = strlen(temp);
	char parsedInstruction[MAX_PARSED_NUM][MAX_PARSED_NUM + 10] = { 0, };
	int count = 0;
	for (int i = 0, j = 0; i < len; i++) {
		if (temp[i] != ' ') {
			parsedInstruction[count][j] = temp[i];
			j++;
		}
		else {
			count++;
			j = 0;
		}
	}
	char* pInstruction = parsedInstruction;
	if (!searchTrie(root, parsedInstruction[0])) {
		return false;
	}
	else {
		if (*pInstruction == 'h') {
			if (*(pInstruction + 1) == 'e' || *(pInstruction + 1) == '\0') {
				current_command = help;
			}
			else if (*(pInstruction + 1) == 'i') {
				current_command = history;
			}
		}
		else if (*pInstruction == 'd' && *(pInstruction + 1) == 'i') {
			current_command = dir;
		}
		else if (*pInstruction == 'q' && *(pInstruction + 1) == 'u') {
			current_command = quit;
			quit_flag = true;
		}
		else if (*pInstruction == 'd' && *(pInstruction + 1) == 'u') {
			current_command = dump;
		}
		else if (*pInstruction == 'e') {
			if (*(pInstruction + 1) == 'd' || *(pInstruction + 1) == '\0') {
				current_command = edit;
			}
		}
		else if (*pInstruction == 'f') {
			if (*(pInstruction + 1) == 'i' || *(pInstruction + 1) == '\0') {
				current_command = fill;
			}
		}
		else if (*pInstruction == 'r' && *(pInstruction + 1) == 'e') {
			current_command = reset;
		}
		else if (!strcmp(parsedInstruction[0], "opcode")) {
			current_command = opcode;
		}
		else if (!strcmp(parsedInstruction[0], "opcodelist")) {
			current_command = opcodelist;
		}
	}
	bool Executable = isExecutable(current_command, parsedInstruction);
	if (!Executable) {
		return false;
	}
	else {
		playCommand(current_command, parsedInstruction);
		return true;
	}
	/*if (!strcmp(instruction, "h") || !strcmp(instruction, "help")) {

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

	}*/
	if (success_flag) {
		insert_queue(instruction);
	}
	return success_flag;

}
