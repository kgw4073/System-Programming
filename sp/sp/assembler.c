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
#include "assembler.h"



RETURN_CODE pass1(char filename[MAX_PARSED_NUM + 10]) {
	char lines[100];
	char label[30], opcode[30], operand[30], overflow[30];
	int starting_address = 0;
	int line = 1;
	char a, b, c, d;
	FILE* fp = fopen(filename, "r");
	
	fgets(lines, 100, fp);
	sscanf(lines, "%s%c%s%c%s%c%s%c", label, &a, opcode, &b, operand, &c, overflow, &d);

	if (strcmp(opcode, "START")) {
		starting_address = 0;
		//return ASSEMBLE_FILE_ERROR;
	}
	else {
		for (int i = strlen(operand) - 1; i >= 0; i--) {
			int next = toDecimal(operand[i]);
			if (next == WRONG_HEXA) {
				return ADDRESS_INPUT_ERROR;
			}
			starting_address += next;
		}
	}
		
	printf("%d %s %s %s\n", line, label, opcode, operand);
	while (1) {
		lines[0] = 0;
		fgets(lines, 100, fp);

		if (lines[0] == '\0') {
			break;
		}
		a = b = c = d = 0;
		sscanf(lines, "%s%c%s%c%s%c", label, &a, opcode, &b, operand, &c, overflow, &d);

		if (label[0] == '.') {
			continue;
		}
		line++;
		if (d != 0) {
			fprintf("Error : Line %d has too many arguments\n", line);
			continue;
		}
		if (b == 0) {
		}
		else if (c == 0) {
			printf("%d %s %s\n", line, label, opcode);
		}
		else if (d == 0) {
			printf("%d %s %s %s\n", line, label, opcode, operand);
		}
		if (!strcmp(label, "END")) {
			break;
		}
	}
}

RETURN_CODE pass2(char filename[MAX_PARSED_NUM + 10]) {
	return NORMAL;
}

void doAssemble(char parsedInstruction[][MAX_PARSED_NUM + 10]) {

	char lst[30];
	char obj[30];
	char temp[] = "lst";
	char temp2[] = "obj";
	strcpy(lst, parsedInstruction[1]);
	strcpy(obj, lst);
	int len = (int)strlen(lst);
	lst[len - 3] = '\0';
	strcat(lst, temp);
	strcat(obj, temp2);

	if (pass1(parsedInstruction[1]) == NORMAL) {
		if (pass2(parsedInstruction[1] == NORMAL)) {

		}
	}
	


}