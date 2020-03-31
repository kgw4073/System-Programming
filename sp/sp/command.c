#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include "command.h"
#include "20150514.h"
#include "BasicCommand.h"

int parameters[3] = { 0, };
int lastAddress = -1;
inline bool isOverflowed(int address) {
	if (address < 0 || address > 0xfffff) return true;
	else return false;
}

inline int toDecimal(char c) {
	if ('0' <= c && c <= '9') return c - '0';
	else if ('a' <= c && c <= 'z') return c - 'a' + 10;
	else if ('A' <= c && c <= 'Z') return c - 'A' + 10;
	else return WRONG_HEXA;
}
void insertHistory(char instruction[MAX_COMMAND_SIZE]) {
	command_list* new_list_node = (command_list*)malloc(sizeof(command_list));

	if (head_of_command_queue == tail_of_command_queue) {
		int len = (int)strlen(instruction);
		head_of_command_queue->command = (char*)malloc((size_t)(len + 1)*sizeof(char));
		strncpy(head_of_command_queue->command, instruction, (size_t)len);
		head_of_command_queue->command[len] = '\0';
		head_of_command_queue->next = new_list_node;
		tail_of_command_queue = new_list_node;

	}
	else {
		int len = (int)strlen(instruction);
		tail_of_command_queue->command = (char*)malloc((size_t)(len + 1) * sizeof(char));
		strncpy(tail_of_command_queue->command, instruction, (size_t)len);
		tail_of_command_queue->command[len] = '\0';
		tail_of_command_queue->next = new_list_node;
		tail_of_command_queue = new_list_node;
	}
}

void dumpMemory(int parameters[], int parsedNumber) {
	int start = (parameters[0] / 16) * 16;
	int end = (parameters[1] / 16) * 16 + 16;
	for (int i = start; i < end; i++) {
		if (i % 16 == 0) {
			printf("%05X ", i);
		}
		if (i >= parameters[0] && i <= parameters[1]) {
			printf("%02X ", vMemory[i]);
		}
		else {
			printf("   ");
		}
		if (i % 16 == 15) {
			printf("; ");
			for (int k = i / MAX_BYTES_LINE * MAX_BYTES_LINE; k < (i / MAX_BYTES_LINE + 1) * MAX_BYTES_LINE; k++) {
				if (vMemory[k] >= 0x20 && vMemory[k] <= 0x7e && k >= parameters[0] && k <= parameters[1]) {
					printf("%c", vMemory[k]);
				}
				else printf(".");
			}
			printf("\n");
		}
	}

}
void deleteHistory() {
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

void showHistory() {
	int count = 1;
	command_list* temp = head_of_command_queue;
	while (temp != tail_of_command_queue) {
		printf("%-4d ", count++);
		printf("%s\n", temp->command);
		temp = temp->next;
	}
}


void showHelp() {
	printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\n\
du[mp][start, end]\ne[dit] address, value\nf[ill] start, end, value\n\
reset\nopcode mnemonic\nopcodelist\n");

}
void showDir() {
	DIR* dp = NULL;
	struct dirent* entry;
	struct stat buf;

	if ((dp = opendir(".")) == NULL) {
		printf("[Error] cannot open ...\n");
		return;
	}
	
	int counter = 0;
	while ((entry = readdir(dp)) != NULL) {
		char filename[100];
		lstat(entry->d_name, &buf);
		//printf(" ");
		strcpy(filename, entry->d_name);

		if (S_ISDIR(buf.st_mode)) {
			if (!strcmp(entry->d_name, "..") || !strcmp(entry->d_name, ".")) {
				;
			}
			else {
				strcat(filename, "/");
			}
		}
		else if((S_IXUSR & buf.st_mode)) {
			strcat(filename, "*");
		}
		printf("%-20s", filename);
		if (++counter % 4 == 0) {
			printf("\n");
		}
	}
	if (counter % 4) printf("\n");
	closedir(dp);
	return;
}

void resetMemory() {
	memset(vMemory, 0, sizeof(vMemory));
}

void editMemory(int parameters[]) {
	vMemory[parameters[0]] = (unsigned char)parameters[1];
}

void fillMemory(int parameters[]) {
	for (int inch = parameters[0]; inch <= parameters[1]; inch++) {
		vMemory[inch] = (unsigned char)parameters[2];
	}
}
OpNode* findOpCodeNode(int OpCodeDecimal) {
	int hashEntry = OpCodeDecimal % MAX_HASH_SIZE;
	OpNode* search = hashTable[hashEntry];
	while (1) {
		if (search == NULL) return NULL;
		if (search->decimal == OpCodeDecimal) {
			return search;
		}
		search = search->next;
	}
	return NULL;
}
void showMnemonic(int parameters[]) {
	int OpCodeDecimal = parameters[0];
	OpNode* show;
	if ((show = findOpCodeNode(OpCodeDecimal)) != NULL) {
		printf("opcode is %02X\n", show->value);
	}
}

void showOpcodelist() {
	for (int i = 0; i < MAX_HASH_SIZE; i++) {
		printf("%d : ", i);
		OpNode* inch = hashTable[i];
		if (inch == hashTailPointer[i]) {
			printf("\n");
			continue;
		}
		printf("[%s,%02X] ", inch->code, inch->value);
		inch = inch->next;
		while (1) {
			if (inch == hashTailPointer[i]) {
				printf("\n");
				break;
			}
			printf("-> ");
			printf("[%s,%02X] ", inch->code, inch->value);
			inch = inch->next;
		}
	}
}
void playCommand(int current_command, char parsedInstruction[][MAX_PARSED_NUM + 10], int parsedNumber) {
	switch (current_command) {
	case help:
		showHelp();
		break;

	case dir:
		showDir();
		break;

	case quit:
		quit_flag = true;
		break;

	case history:
		showHistory();
		break;

	case dump:
		dumpMemory(parameters, parsedNumber);
		break;

	case edit:
		editMemory(parameters);
		break;

	case fill:
		fillMemory(parameters);
		break;

	case reset:
		resetMemory();
		break;

	case opcode:
		showMnemonic(parameters);
		break;

	case opcodelist:
		showOpcodelist();
		break;

	}
}

RETURN_CODE isExecutable(enum input_command current_command, 
	char parsedInstruction[][MAX_PARSED_NUM + 10], int* parsedReference) {

	//RETURN_CODE ret = NORMAL;
	int start = 0, last = 0, valueDecimal = 0, hexaDecimal = 0;
	int parsedNumber = 0;
	for (int i = 0; ; i++) {
		if (!parsedInstruction[i][0]) break;
		parsedNumber++;
	}
	if (current_command == dump) {
		hexaDecimal = 0;
		int lastIndex = (int)strlen(parsedInstruction[1]) - 1;

		if (parsedNumber == 1) {
			start = lastAddress + 1;
			if (start > 0xfffff) start = 0;
			lastAddress = (start + MAX_DUMP_BYTE * MAX_DUMP_LINE - 1);
			if (lastAddress >= 0xfffff) lastAddress = 0xfffff;
			last = lastAddress;
			parameters[0] = start, parameters[1] = last, * parsedReference = parsedNumber;
		}

		else if (parsedNumber == 2) {
			// 인자가 두 개이거나 더 이상 파싱할 필요가 없을 경우
			for (int i = lastIndex, j = 0; i >= 0; i--, j++) {
				if ((hexaDecimal = toDecimal(parsedInstruction[1][i])) == WRONG_HEXA) {
					return ADDRESS_INPUT_ERROR;
				}
				else {
					start += (int)pow(16, (double)j) * hexaDecimal;
				}
			}
			// hexa가 제대로 들어오긴 했을 때
			if (isOverflowed(start)) {
				return MEMORY_INDEX_ERROR;
			}
			else {
				last = start + MAX_DUMP_BYTE * MAX_DUMP_LINE - 1;
				if (last >= 0xfffff) last = 0xfffff;
				lastAddress = last;
			}


		}
		else if (parsedNumber == 3) {
			// 명령어가 세 부분으로 이루어 지면 두 번째 인자 마지막에 ','를 포함해야 한다.

			if (parsedInstruction[1][lastIndex] == ',') {
				for (int i = lastIndex - 1, j = 0; i >= 0; i--, j++) {
					if ((hexaDecimal = toDecimal(parsedInstruction[1][i])) == WRONG_HEXA) {
						return ADDRESS_INPUT_ERROR;
					
					}
					else {
						start += (int)pow(16, (double)j) * hexaDecimal;
					}
				}
			}
			else {
				return COMMAND_ERROR;
			}

			lastIndex = (int)strlen(parsedInstruction[2]) - 1;
			for (int i = lastIndex, j = 0; i >= 0; i--, j++) {
				if ((hexaDecimal = toDecimal(parsedInstruction[2][i])) == WRONG_HEXA) {
						
					return ADDRESS_INPUT_ERROR;
				}
				else {
					last += (int)pow(16, (double)j) * hexaDecimal;
				}
			}
			if (isOverflowed(start) || isOverflowed(last) || start > last) {
				return MEMORY_INDEX_ERROR;
			}
			else {
				lastAddress = last;
			}
		}
		// 명령어 파트가 더 많이 들어온 경우
		else {
			return COMMAND_ERROR;
		}

		parameters[0] = start, parameters[1] = last, * parsedReference = parsedNumber;
		
	}
	else if (current_command == edit) {
		if (parsedNumber != 3) {
			return COMMAND_ERROR;
		}
		else {
			hexaDecimal = 0;
			int lastIndex = (int)strlen(parsedInstruction[1]) - 1;
			// 인자가 두 개이거나 더 이상 파싱할 필요가 없을 경우
			if (parsedInstruction[1][lastIndex] == ',') {
				for (int i = lastIndex - 1, j = 0; i >= 0; i--, j++) {
					if ((hexaDecimal = toDecimal(parsedInstruction[1][i])) == WRONG_HEXA) {
						return ADDRESS_INPUT_ERROR;
					}
					else {
						start += (int)pow(16, (double)j) * hexaDecimal;
					}
				}
			}
			else {
				
				return COMMAND_ERROR;
			}

			lastIndex = (int)strlen(parsedInstruction[2]) - 1;
			for (int i = lastIndex, j = 0; i >= 0; i--, j++) {
				if ((hexaDecimal = toDecimal(parsedInstruction[2][i])) == WRONG_HEXA) {
					return ADDRESS_INPUT_ERROR;
				}
				else {
					valueDecimal += (int)pow(16, (double)j) * hexaDecimal;
				}
			}
			if (isOverflowed(start)) {
				return MEMORY_INDEX_ERROR;
			}
			if (valueDecimal < 0 || valueDecimal > 0xff) {
				return VALUE_ERROR;
			}
				
			parameters[0] = start, parameters[1] = valueDecimal;

		}

	}

	else if (current_command == fill) {
		if (parsedNumber != 4) {
			return COMMAND_ERROR;
		}
		else {
			int hexaDecimal = 0;
			if (parsedNumber == 1) {
				start = lastAddress;
				lastAddress += MAX_DUMP_BYTE * MAX_DUMP_LINE;

			}
			int lastIndex = (int)strlen(parsedInstruction[1]) - 1;
			if (parsedInstruction[1][lastIndex] == ',') {
				for (int i = lastIndex - 1, j = 0; i >= 0; i--, j++) {
					if ((hexaDecimal = toDecimal(parsedInstruction[1][i])) == WRONG_HEXA) {
						return ADDRESS_INPUT_ERROR;
					}
					else {
						start += (int)pow(16, (double)j) * hexaDecimal;
					}
				}
			}
			else {
				return COMMAND_ERROR;
			}

			lastIndex = (int)strlen(parsedInstruction[2]) - 1;
			if (parsedInstruction[2][lastIndex] == ',') {
				for (int i = lastIndex - 1, j = 0; i >= 0; i--, j++) {
					if ((hexaDecimal = toDecimal(parsedInstruction[2][i])) == WRONG_HEXA) {
						return ADDRESS_INPUT_ERROR;
						
					}
					else {
						last += (int)pow(16, (double)j) * hexaDecimal;
					}
				}
			}
			else {
				return COMMAND_ERROR;
			}

			lastIndex = (int)strlen(parsedInstruction[3]) - 1;
			for (int i = lastIndex, j = 0; i >= 0; i--, j++) {
				if ((hexaDecimal = toDecimal(parsedInstruction[3][i])) == WRONG_HEXA) {
					return ADDRESS_INPUT_ERROR;
						
				}
				else {
					valueDecimal += (int)pow(16, (double)j) * hexaDecimal;
				}
			}
			if (isOverflowed(start) || isOverflowed(last) || start > last) {
				return MEMORY_INDEX_ERROR;
			}
			if (valueDecimal < 0 || valueDecimal > 0xff) {
				return VALUE_ERROR;
			}
			
			parameters[0] = start, parameters[1] = last, parameters[2] = valueDecimal;
			
		}

	}

	else if (current_command == opcode) {
		if (parsedNumber != 2) return false;
		int len = (int)(int)strlen(parsedInstruction[1]);
		int base = 1;
		int OpCodeDecimal = 0;
		for (int i = 0; i < len; i++) {
			int decimal = parsedInstruction[1][i] - 'A';
			if (decimal >= 0 && decimal <= 26) {
				OpCodeDecimal += (decimal * base);
				base *= 26;
			}
			else {
				return OPCODE_ERROR;
			}
		}
		
		parameters[0] = OpCodeDecimal;
		return NORMAL;

	}

	else {
		if (parsedNumber != 1) return COMMAND_ERROR;
	}
	*parsedReference = parsedNumber;
	return NORMAL;
}

void commandParse(char instruction[MAX_COMMAND_SIZE]) {
	char temp[MAX_COMMAND_SIZE];
	RETURN_CODE ret;
	int parsed_length = 0;
	

	bool blank_buffer = false;
	for (int i = 0; i < MAX_COMMAND_SIZE; i++) {
		if (instruction[i] == ' ') {
			if (!blank_buffer && parsed_length > 0) {
				blank_buffer = true;
				temp[parsed_length++] = instruction[i];
			}
		}
		else if (instruction[i] == '\0') {
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



	int len = (int)strlen(temp);
	char parsedInstruction[MAX_PARSED_NUM][MAX_PARSED_NUM + 10];
	memset(parsedInstruction, 0, sizeof(parsedInstruction));
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
	char* pInstruction = &parsedInstruction[0][0];

	if (!searchTrie(root, parsedInstruction[0])) {
		ret = COMMAND_ERROR;
		goto ERROR_HANDLING;
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

		else if (*pInstruction == 'd') {
			if (*(pInstruction + 1) == 'i' || *(pInstruction + 1) == '\0') {
				current_command = dir;
			}
			if (*(pInstruction + 1) == 'u') {
				current_command = dump;
			}
		}

		else if (*pInstruction == 'q' && *(pInstruction + 1) == 'u') {
			current_command = quit;
			quit_flag = true;
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
	int parsedNumber = 0;
	ret = isExecutable(current_command, parsedInstruction, &parsedNumber);


ERROR_HANDLING:
	switch (ret) {
	case NORMAL:
		insertHistory(instruction);
		playCommand(current_command, parsedInstruction, parsedNumber);
		break;

	case ADDRESS_INPUT_ERROR:
		STDERR_ADDRESS_ERROR();
		break;

	case MEMORY_INDEX_ERROR:
		STDERR_MEMORY_CORRUPT();
		break;

	case VALUE_ERROR:
		STDERR_VALUE_ERROR();
		break;

	case COMMAND_ERROR:
		STDERR_COMMAND_ERROR();
		break;

	case OPCODE_ERROR:
		STDERR_OPCODE_ERROR();
		break;
	}
}
