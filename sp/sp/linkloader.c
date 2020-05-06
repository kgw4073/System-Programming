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
#include "linkloader.h"

void link_and_load(char parsedInstruction[][MAX_PARSED_NUM + 10]) {
	sizeOfBP = 0;

}
void BreakPoint(int parameters[], char str[]) {
	if (parameters[0] == CLEAR_BP_FLAG) {
		sizeOfBP = 0;
		printf("            [ok] clear all breakpoints\n");
	}
	else if (parameters[0] == PRINT_BP_FLAG) {
		printf("            breakpoint\n");
		printf("            ----------\n");
		for (int i = 0; i < sizeOfBP; i++) {
			printf("            %0X\n", BPlist[i]);
		}
	}
	else {
		BPlist[sizeOfBP++] = parameters[0];
		printf("            [ok] create breakpoint %s\n", str);
	}
}

void setProgaddr(int parameters[]) {
	Progaddr = parameters[0];
}