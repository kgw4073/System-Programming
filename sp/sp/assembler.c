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
ProgramCounter = 0, BaseAddress = 0;

Symbol* getNewSymbolNode() {
	Symbol* temp = (Symbol*)malloc(sizeof(Symbol));
	temp->loc = -1;
	temp->left = temp->right = NULL;
	return temp;
}

Symbol* findBinaryTreeSymbol(Symbol* root, char symbol[20]) {
	if (root) {
		int res = strcmp(root->symbol, symbol);
		if (res < 0) {
			return findBinaryTreeSymbol(root->left, symbol);
		}
		else if (res > 0) {
			return findBinaryTreeSymbol(root->right, symbol);
		}
		else return root;
	}
	else return NULL;
}
bool insertBinaryTreeSymbol(Symbol* root, char symbol[20], int loc) {

	if (root->loc == -1) {
		strcpy(root->symbol, symbol);
		if (!strcmp(symbol, BaseSymbol)) {
			BaseAddress = loc;
		}
		root->loc = loc;
		Symbol* temp = getNewSymbolNode();
		Symbol* temp2 = getNewSymbolNode();
		root->left = temp, root->right = temp2;
		return true;
	}
	int res = strcmp(root->symbol, symbol);

	if (res < 0) {
		return insertBinaryTreeSymbol(root->left, symbol, loc);
	}
	else if (res > 0) {
		return insertBinaryTreeSymbol(root->right, symbol, loc);
	}
	else {
		return false;
	}
}
REG findRegNumber(char str[]) {
	if (!strcmp(str, "A")) {
		return A;
	}
	else if (!strcmp(str, "X")) {
		return X;
	}
	else if (!strcmp(str, "L")) {
		return L;
	}
	else if (!strcmp(str, "B")) {
		return B;
	}
	else if (!strcmp(str, "S")) {
		return S;
	}
	else if (!strcmp(str, "T")) {
		return T;
	}
	else if (!strcmp(str, "F")) {
		return F;
	}
	else if (!strcmp(str, "PC")) {
		return PC;
	}
	else if (!strcmp(str, "SW")) {
		return SW;
	}
	else return -1;
}

void objectCodeWrite(int line, int loc, int parts, bool isFormat4, OpNode* search, int constant, char part1[], char part2[], char part3[], char part4[], bool labelFlag) {
	int l = line / 5;
	objectCode[l].format = search->format + isFormat4;
	objectCode[l].loc = loc;
	int format = objectCode[l].format;
	int code = 0;
	
	if (constant) {

		return;
	}

	// Label있음
	if (labelFlag == true) {
		strcpy(objectCode[l].label, part1);
		strcpy(objectCode[l].opcode, part2);
		strcpy(objectCode[l].operand_first, part3);
		strcpy(objectCode[l].operand_second, part4);
		

		if (parts == 2) {
			if (format == 1) {
				code = search->value;
			}
			else if (format == 2) {
				code |= (search->value << 8);
			}
			else if (format == 3) {
				code |= (search->value << 18);
				// simple addressing
				code |= (3 << 16);
			}
			else if (format == 4) {
				code |= (search->value << 26);
			}
		}
		else if (parts == 3) {
			// opcode(8) reg1(4) reg2(4)
			if (format == 2) {
				REG num = findRegNumber(part3);
				code |= (search->value << 8);
				code |= (num << 4);
			}
			// opcode(6) nixbpe(6) disp(12)
			else if (format == 3) {
				code |= (search->value << 18);

			}
		}
		else if (parts == 4) {

		
		}
		else {

		}

	}
	else {
		strcpy(objectCode[l].opcode, part1);
		strcpy(objectCode[l].operand_first, part2);
		strcpy(objectCode[l].operand_second, part3);

	}
}

bool pass1(char filename[MAX_PARSED_NUM + 10]) {
	char lines[100];
	char part1[30], part2[30], part3[30], part4[30], part5[30];
	char Label[30], Opcode[30], Operand_first[30], Operand_second[30];
	int starting_address = 0, locctr = 0;
	int line = 5;
	bool isFormat4 = false;
	OpNode* search, *search_first, * search_second;
	char a, b, c, d, e;
	//Formats binaryCode
	FILE* fp = fopen(filename, "r");
	//FILE* intermediate = fopen(lst, "w");



	fgets(lines, 100, fp);
	sscanf(lines, "%s%c%s%c%s%c%s%c", part1, &a, part2, &b, part3, &c, part4, &d, part5, &e);
	bool errorFlag = false;
	if (!strcmp(part2, "START")) {
		for (int i = strlen(part3) - 1; i >= 0; i--) {
			int next = toDecimal(part3[i]);
			if (next == WRONG_HEXA) {
				objectCode[line / 5].ret = ADDRESS_INPUT_ERROR;
				errorFlag = true;
				continue;
			}
			starting_address += next;
		}
		locctr = starting_address;
	}
	else {
		starting_address = 0;

	}
	//fprintf(intermediate, "%7s%7s%7s\n", part1, part2, part3);
	objectCode[line / 5].format = -1;
	printf("%7s%7s%7s\n", part1, part2, part3);
	//printf("%d %s %s %s\n", line, label, opcode, operand);
	while (1) {
		bool isFormat4 = false;

		lines[0] = 0;
		fgets(lines, 100, fp);
		Label[0] = Opcode[0] = Operand_first[0] = Operand_first[0] = '\0';
		if (lines[0] == '\0') {
			break;
		}
		
		//fprintf(intermediate, "%-7d", line);
		line += 5;
		a = b = c = d = 0;
		part1[0] = part2[0] = part3[0] = part4[0] = part5[0] = '\0';
		sscanf(lines, "%s%c%s%c%s%c", part1, &a, part2, &b, part3, &c, part4, &d, part5, &e);
		objectCode[line / 5].ret = NORMAL;
		objectCode[line / 5].format = -1;

		
		if (part1[0] == '.') {
			continue;
		}
		if (!strcmp(part1, "END")) {
			




			printf("%-7d              %-7s%-7s\n", line, part1, part2);

			break;
		}
		if (!strcmp(part1, "BASE")) {


			strcpy(BaseSymbol, part2);
			

			printf("%-7d        %-7s%-7s\n", line, part1, part2);
			//line += 5;
			continue;
		}
		int parts_num = 0;
		// Label 있음
		if (lines[0] != ' ') {
			// 이미 Label이 테이블에 있으면 에러
			if (!insertBinaryTreeSymbol(symbolRoot, part1, locctr)) {
				objectCode[line / 5].ret = DUPLICATE_SYMBOL_ERROR;
				errorFlag = true;
				continue;
			}
			memcpy(Label, part1, sizeof(part1));

		}


		// 하나
		if (a == '\n') {
			if (Label[0] != '\0') {
				//fprintf(inter, "%d %d %d\n", OPCODE_ERROR, line, locctr);
				objectCode[line / 5].ret = OPCODE_ERROR;
				errorFlag = true;
				continue;
			}
			
			search = searchHashNode(part1, strlen(part1));
			if (!search) {
				objectCode[line / 5].ret = OPCODE_ERROR;
				errorFlag = true;
				continue;
				
			}
			///##########################
			ProgramCounter = locctr + search->format;
			objectCodeWrite(line, locctr, 1, isFormat4, search, 1, part1, part2, part3, part4, false);
			printf("%04x ", ProgramCounter);
			printf("%-7d%04X        %-7s\n", line, locctr, part1);
			locctr = ProgramCounter;

		}
		// 파트 두 개
		else if (b == '\n') {
			if (Label[0] == '\0') {
				memcpy(Opcode, part1, sizeof(part1));
				memcpy(Operand_first, part2, sizeof(part2));
			}
			else {
				memcpy(Operand_first, part2, sizeof(part2));
			}

			// format 4
			if (Opcode[0] == '+') {
				search = searchHashNode(Opcode + 1, strlen(Opcode + 1));
				if (!search) {
					objectCode[line / 5].ret = OPCODE_ERROR;
					errorFlag = true;
					continue;
					
				}
				isFormat4 = true;
				// #########################


				
			}
			else {
				search = searchHashNode(Opcode, strlen(Opcode));
				if (!search) {
					objectCode[line / 5].ret = OPCODE_ERROR;
					errorFlag = true;
					continue;
					
				}
				// #########################

			}
			ProgramCounter = (locctr + search->format + isFormat4);
			printf("%04x ", ProgramCounter);
			if (Label[0] == '\0') {
				printf("%-7d%04X       %-7s%-7s\n", line, locctr, Opcode, Operand_first);
				objectCodeWrite(line, locctr, 2, isFormat4, search, 0, part1, part2, part3, part4, false);

			}
			//Label
			else {
				printf("%-7d%04X%-7s%-7s\n", line, locctr, Label, Opcode);
				objectCodeWrite(line, locctr, 2, isFormat4, search, 0, part1, part2, part3, part4, true);

			}
			locctr = ProgramCounter;

		}
		// 파트 세 개
		else if (c == '\n') {
			if (Label[0] == '\0') {
				memcpy(Opcode, part1, sizeof(part1));
				memcpy(Operand_first, part2, sizeof(part2));
				memcpy(Operand_second, part3, sizeof(part3));

			}
			// Label이 있는 위치에 뭔가 있을 때.
			else {
				bool varFlag = false;
				if (!strcmp(part2, "BYTE")) {
					varFlag = true;
					if (part3[0] == 'C') {
						int sz = (int)strlen(part3);
						if (part3[1] == '\'' && part3[sz - 1] == '\'') {
							
							ProgramCounter = locctr + (sz - 3);
							printf("%04x ", ProgramCounter);
							objectCodeWrite(line, locctr, 3, false, search, 1, part1, part2, part3, part4, false);

							printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, part1, part2, part3);
							locctr = ProgramCounter;
						}
						else {
							objectCode[line / 5].ret = OPERAND_ERROR;
							errorFlag = true;
							continue;
							
						}

					}
					else if (part3[0] == 'X') {
						int byte = toDecimal(part3[3]) + 16 * toDecimal(part3[2]);
						ProgramCounter++;
						printf("%04x ", ProgramCounter);
						objectCodeWrite(line, locctr, 3, false, search, 1, part1, part2, part3, part4, false);

						printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, part1, part2, part3);

						locctr = ProgramCounter;
					}
					else {
						objectCode[line / 5].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}

				}
				else if (!strcmp(part2, "RESB")) {
					varFlag = true;

					int size = atoi(part3);
					if (size == 0) {
						objectCode[line / 5].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
					
					printf("    ");
					printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, part1, part2, part3);
					ProgramCounter = locctr + size;
					locctr = ProgramCounter;
				}
				else if (!strcmp(part2, "WORD")) {
					varFlag = true;
					ProgramCounter = locctr + 3;
					printf("%04x ", ProgramCounter);
					objectCodeWrite(line, locctr, 3, false, search, 1, part1, part2, part3, part4, false);
					printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, part1, part2, part3);
					locctr = ProgramCounter;

				}
				else if (!strcmp(part2, "RESW")) {
					varFlag = true;

					int size = atoi(part3);
					if (size == 0) {
						objectCode[line / 5].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
					
					printf("     ");
					printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, part1, part2, part3);
					ProgramCounter = locctr + (3 * size);
					locctr = ProgramCounter;

				}
				if (varFlag) {
					if (!insertBinaryTreeSymbol(symbolRoot, part1, locctr)) {
						objectCode[line / 5].ret = DUPLICATE_SYMBOL_ERROR;
						errorFlag = true;
						continue;
					}
					continue;
				}
				
				memcpy(Opcode, part2, sizeof(part2));
				memcpy(Operand_first, part3, sizeof(part3));

			}
			// format 4
			if (Opcode[0] == '+') {
				isFormat4 = true;
				search = searchHashNode(Opcode + 1, strlen(Opcode + 1));
			}
			else {
				search = searchHashNode(Opcode, strlen(Opcode));
			}
			if (!search) {
				objectCode[line / 5].ret = OPERAND_ERROR;
				errorFlag = true;
				continue;
			}
			// #########################

			// Label 없음
			ProgramCounter = locctr + (search->format + isFormat4);
			printf("%04x ", ProgramCounter);
			if (Label[0] == '\0') {
				objectCodeWrite(line, locctr, 3, isFormat4, search, 0, part1, part2, part3, part4, false);
				printf("%-7d%04X       %-7s%-7s%-7s\n", line, locctr, Opcode, Operand_first, Operand_second);
			}
			// Label 있음
			else {
				objectCodeWrite(line, locctr, 3, isFormat4, search, 0, part1, part2, part3, part4, true);
				printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, Label, Opcode, Operand_first);
			}
			locctr = ProgramCounter;


		}
		// 파트 네 개
		else if (d == '\n') {

			if (Label[0] != '\0') {
				memcpy(Opcode, part2, sizeof(part2));
				memcpy(Operand_first, part3, sizeof(part3));
				memcpy(Operand_second, part4, sizeof(part4));
			}
			else {
				objectCode[line / 5].ret = OPCODE_ERROR;
				errorFlag = true;
				continue;

			}
			if (Opcode[0] == '+') {
				isFormat4 = true;
				search = searchHashNode(Opcode + 1, strlen(Opcode + 1));
			}
			else {
				search = searchHashNode(Opcode, strlen(Opcode));
			}
			ProgramCounter = locctr + (search->format + isFormat4);
			printf("%04x ", ProgramCounter);
			objectCodeWrite(line, locctr, 4, isFormat4, search, 0, part1, part2, part3, part4, true);
			locctr = ProgramCounter;
		}
		// 파트 다섯 개
		else {
			objectCode[line / 5].ret = OPCODE_ERROR;
			errorFlag = true;
			continue;
		}
	}
	printf("%04X\n", BaseAddress);
	fclose(fp);
	return errorFlag;


}

bool pass2(char filename[MAX_PARSED_NUM + 10]) {


	return NORMAL;
}

void doAssemble(char parsedInstruction[][MAX_PARSED_NUM + 10]) {


	char temp[] = "lst";
	char temp2[] = "obj";
	strcpy(lst, parsedInstruction[1]);
	strcpy(obj, lst);
	int len = (int)strlen(lst);
	lst[len - 3] = '\0';
	strcat(lst, temp);
	strcat(obj, temp2);

	symbolRoot = getNewSymbolNode();

	if (pass1(parsedInstruction[1]) == NORMAL) {
		if (pass2(parsedInstruction[1] == NORMAL)) {

		}
	}
	


}