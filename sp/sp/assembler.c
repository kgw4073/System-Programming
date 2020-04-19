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
int ProgramCounter = 0, BaseAddress = 0;

Symbol* getNewSymbolNode() {
	Symbol* temp = (Symbol*)malloc(sizeof(Symbol));
	temp->symbol[0] = '\0';
	temp->loc = -1;
	temp->left = temp->right = NULL;
	return temp;
}

Symbol* findBinaryTreeSymbol(Symbol* root, char symbol[20]) {
	if (root) {
		int res = strcmp(root->symbol, symbol);
		if (res > 0) {
			return findBinaryTreeSymbol(root->left, symbol);
		}
		else if (res < 0) {
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
		root->left = root->right = NULL;
		return true;
	}
	int res = strcmp(root->symbol, symbol);

	if (res > 0) {
		if (!root->left) {
			Symbol* temp = getNewSymbolNode();
			root->left = temp;
		}
		return insertBinaryTreeSymbol(root->left, symbol, loc);
	}
	else if (res < 0) {
		if (!root->right) {
			Symbol* temp = getNewSymbolNode();
			root->right = temp;
		}
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
	objectCode[l].op = search->value;
	
	int format = objectCode[l].format;

	if (constant) {
		strcpy(objectCode[l].label, part1);
		strcpy(objectCode[l].opcode, part2);
		strcpy(objectCode[l].operand_first, part3);
		objectCode[l].isConstant = true;
		return;
	}
	
	// Label있음
	if (labelFlag == true) {
		strcpy(objectCode[l].label, part1);
		strcpy(objectCode[l].opcode, part2);
		strcpy(objectCode[l].operand_first, part3);
		strcpy(objectCode[l].operand_second, part4);
		
		objectCode[l].parts = parts - 1;
		if (parts == 2) {
			if (format == 2) {
				REG num = findRegNumber(part3);
				objectCode[l].reg1 = num;
				num = findRegNumber(part4);
				if (num != -1) {
					objectCode[l].reg2 = num;
				}
				else {
					objectCode[l].reg2 = 0;
				}
			}
			else if (format == 3) {
				// indirect addressing
				if (part3[0] == '@') {
					objectCode[l].nixbpe |= (2 << 4);
				}
				// immediate addressing
				else if (part3[0] == '#') {
					if (!strcmp(part3 + 1, "0")) {
						objectCode[l].disp = 0;
					}
					else {
						int disp = atoi(part3 + 1);
						if (disp != 0 && 0 < disp && disp < 0x1000) {
							objectCode[l].disp = disp;
						}
						else objectCode[l].ret = OPERAND_ERROR;
					}
				}

			}

		}
		else if (parts == 3) {
			// opcode(8) reg1(4) reg2(4)
			if (format == 2) {
				REG num = findRegNumber(part3);
				objectCode[l].reg1 = num;
				num = findRegNumber(part4);
				if (num != -1) {
					objectCode[l].reg2 = num;
				}
				else {
					objectCode[l].reg2 = 0;
				}
			}
			// opcode(6) nixbpe(6) disp(12)
			else if (format == 3) {
				// indirect addressing
				if (part3[0] == '@') {
					objectCode[l].nixbpe |= (2 << 4);
				}
				// immediate addressing
				else if (part3[0] == '#') {
					if (!strcmp(part3 + 1, "0")) {
						objectCode[l].disp = 0;
					}
					else {
						int disp = atoi(part3 + 1);
						if (disp != 0 && 0 < disp && disp < 0x1000) {
							objectCode[l].disp = disp;
						}
						else objectCode[l].ret = OPERAND_ERROR;
					}
				}
				
			}
			else if (format == 4) {
				if (part3[0] == '@') {
					objectCode[l].nixbpe |= (2 << 4);
					
				}
				// immediate addressing
				else if (part3[0] == '#') {
					if (!strcmp(part3 + 1, "0")) {
						objectCode[l].disp = 0;
					}
					else {
						int addr = atoi(part3 + 1);
						if (addr != 0) {
							objectCode[l].addr = addr;
						}
						else objectCode[l].ret = OPERAND_ERROR;
					}
				}
			}
		}
		else if (parts == 4) {
			
		}
		else {
			objectCode[l].ret = OPERAND_ERROR;
		}

	}
	// Label 없음
	else {
		strcpy(objectCode[l].opcode, part1);
		strcpy(objectCode[l].operand_first, part2);
		strcpy(objectCode[l].operand_second, part3);
		objectCode[l].parts = parts;

		if (parts == 1) {
			;
		}
		else if (parts == 2) {
			if (format == 2) {
				REG num = findRegNumber(part2);
				if (num != -1) {
					objectCode[l].reg1 = num;
				}
				else objectCode[l].ret = OPERAND_ERROR;
			}
			else if (format == 3) {
				if (part2[0] == '@') {
					objectCode[l].nixbpe |= (2 << 4);
				}
				// immediate addressing
				else if (part2[0] == '#') {
					objectCode[l].nixbpe |= (1 << 4);
				}
				else {
					objectCode[l].nixbpe |= (3 << 4);
				}
			}
			else if (format == 4) {
				objectCode[l].nixbpe |= 49;
			}
			else {
				objectCode[l].ret = OPCODE_ERROR;
			}
		}
		else if (parts == 3) {

			// opcode(8) reg1(4) reg2(4)
			if (format == 2) {
				char tmp[30];
				strcpy(tmp, part2);
				int tmplen;
				if (tmp[tmplen = (strlen(part2) - 1)] == ',') {
					tmp[tmplen] = '\0';
				}
				REG num = findRegNumber(tmp);
				objectCode[l].reg1 = num;
				num = findRegNumber(part3);
				if (num != -1) {
					objectCode[l].reg2 = num;
				}
				else {
					objectCode[l].reg2 = 0;
				}
			}
			// opcode(6) nixbpe(6) disp(12)
			else if (format == 3) {
				// indirect addressing
				/*char tmp[30];
				strcpy(tmp, part2);
				int tmplen;
				if (tmp[(tmplen = (strlen(part2) - 1))] == ',') {
					tmp[tmplen] = '\0';
				}*/
				if (!strcmp(part3, "X")) {
					objectCode[l].nixbpe |= (7 << 3);
				}

				

			}
			else if (format == 4) {
				objectCode[l].nixbpe = 49;
			}
		}
		else {
			objectCode[l].ret = OPERAND_ERROR;
		}
	}
}

void deleteSymbolTree(Symbol* root) {
	if (root) {
		deleteSymbolTree(root->left);
		deleteSymbolTree(root->right);
		free(root);
	}
}

bool pass1(char filename[MAX_PARSED_NUM + 10]) {
	char lines[100];
	char part1[30], part2[30], part3[30], part4[30], part5[30];
	char Label[30], Opcode[30], Operand_first[30], Operand_second[30];
	int starting_address = 0, locctr = 0;
	int line = 0;
	//bool isFormat4 = false;
	OpNode* search;
	char a, b, c, d, e;
	//Formats binaryCode
	FILE* fp = fopen(filename, "r");
	//FILE* intermediate = fopen(lst, "w");

	deleteSymbolTree(symbolRoot);
	symbolRoot = getNewSymbolNode();
	

	//printf("%d %s %s %s\n", line, label, opcode, operand);
	bool start_flag = false;
	bool errorFlag = false;
	int idx;
	while (1) {
		bool isFormat4 = false;

		lines[0] = 0;
		fgets(lines, 100, fp);
	
		Label[0] = '\0';
		Opcode[0] = '\0';
		Operand_first[0] = '\0';
		Operand_first[0] = '\0';
		if (lines[0] == '\0') {
			break;
		}
		
		line += 5;
		a = b = c = d = e = 0;
		part1[0] = part2[0] = part3[0] = part4[0] = part5[0] = '\0';
		idx = line / 5;
		sscanf(lines, "%s%c%s%c%s%c%s%c%s%c", part1, &a, part2, &b, part3, &c, part4, &d, part5, &e);

		if (!start_flag) {

			if (!strcmp(part2, "START")) {
				for (int i = (int)strlen(part3) - 1; i >= 0; i--) {
					int next = toDecimal(part3[i]);
					if (next == WRONG_HEXA) {
						objectCode[idx].ret = ADDRESS_INPUT_ERROR;
						errorFlag = true;
						continue;
					}
				}
				if (errorFlag) continue;
				starting_address += strtol(part3, NULL, 16);
				locctr = starting_address;
				objectCode[idx].ret = NORMAL;
				objectCode[idx].format = 0;
				start_flag = true;
				strcpy(objectCode[idx].label, part1);
				strcpy(objectCode[idx].opcode, part2);
				strcpy(objectCode[idx].operand_first, part3);
				//printf("%d %04X %s %s %s\n", line, locctr, part1, part2, part3);
				continue;
			}
			else {
				if (lines[0] != '.') {
					objectCode[idx].ret = OPCODE_ERROR;
					//printf("%d %04X %s", line, locctr, lines);
				}
				continue;
			}
		}


		objectCode[idx].ret = NORMAL;

		if (a == '\0') {
			objectCode[idx].isComment = true;
			objectCode[idx].comment = (char*)malloc(sizeof(char) * 2);
			objectCode[idx].comment[0] = '\n';
			objectCode[idx].comment[1] = '\0';
			continue;
		}
		if (part1[0] == '.') {
			objectCode[idx].isComment = true;
			objectCode[idx].comment = (char*)malloc(sizeof(char) * 100);
			strcpy(objectCode[idx].comment, lines);
			
			//printf("%d %04X %s", line, locctr, lines);
			continue;
		}
		else if (part1[0] == '\0') {
			objectCode[idx].isComment = true;
			objectCode[idx].comment = (char*)malloc(sizeof(char));
			objectCode[idx].comment[0] = '\0';
			//printf("%d %04X %s", line, locctr, lines);
			continue;
		}
		if (!strcmp(part1, "END")) {
			
			objectCode[idx].isDirective = true;
			strcpy(objectCode[idx].opcode, part1);
			strcpy(objectCode[idx].operand_first, part2);
			//printf("%-7d              %-7s%-7s\n", line, part1, part2);

			break;
		}
		if (!strcmp(part1, "BASE")) {


			strcpy(BaseSymbol, part2);
			strcpy(objectCode[idx].opcode, part1);
			strcpy(objectCode[idx].operand_first, part2);
			objectCode[idx].isDirective = true;

			//printf("%-7d        %-7s%-7s\n", line, part1, part2);
			//line += 5;
			continue;
		}
		
		//int parts_num = 0;
		// Label 있음
		if (lines[0] != ' ') {
			// 이미 Label이 테이블에 있으면 에러
			if (!insertBinaryTreeSymbol(symbolRoot, part1, locctr)) {
				objectCode[idx].ret = DUPLICATE_SYMBOL_ERROR;
				errorFlag = true;
				continue;
			}
			memcpy(Label, part1, sizeof(part1));


		}


		// 하나
		if (b == 0) {
			if (Label[0] != '\0') {
				//fprintf(inter, "%d %d %d\n", OPCODE_ERROR, line, locctr);
				objectCode[idx].ret = OPCODE_ERROR;
				errorFlag = true;
				continue;
			}
			
			search = searchHashNode(part1, (int)strlen(part1));
			if (!search) {
				objectCode[idx].ret = OPERAND_NOT_FOUND_ERROR;
				errorFlag = true;
				continue;
				
			}
			///##########################
			ProgramCounter = locctr + search->format;
			objectCodeWrite(line, locctr, 1, isFormat4, search, 0, part1, part2, part3, part4, false);
			//printf("%04x ", ProgramCounter);
			//printf("%-7d%04X        %-7s\n", line, locctr, part1);
			locctr = ProgramCounter;

		}
		// 파트 두 개
		else if (c == 0) {
			if (Label[0] == '\0') {
				memcpy(Opcode, part1, sizeof(part1));
				memcpy(Operand_first, part2, sizeof(part2));
			}
			else {
				memcpy(Operand_first, part2, sizeof(part2));
			}

			// format 4
			if (Opcode[0] == '+') {
				search = searchHashNode(Opcode + 1, (int)strlen(Opcode + 1));
				if (!search) {
					objectCode[idx].ret = OPERAND_NOT_FOUND_ERROR;
					errorFlag = true;
					continue;
					
				}
				isFormat4 = true;
				// #########################
			}
			else {
				search = searchHashNode(Opcode, (int)strlen(Opcode));
				if (!search) {
					objectCode[idx].ret = OPERAND_NOT_FOUND_ERROR;
					errorFlag = true;
					continue;
					
				}
				// #########################
			}
			ProgramCounter = (locctr + search->format + isFormat4);
			//printf("%04x ", ProgramCounter);
			if (Label[0] == '\0') {
				//printf("%-7d%04X       %-7s%-7s\n", line, locctr, Opcode, Operand_first);
				objectCodeWrite(line, locctr, 2, isFormat4, search, 0, part1, part2, part3, part4, false);

			}
			//Label
			else {
				//printf("%-7d%04X%-7s%-7s\n", line, locctr, Label, Opcode);
				objectCodeWrite(line, locctr, 2, isFormat4, search, 0, part1, part2, part3, part4, true);

			}
			locctr = ProgramCounter;

		}
		// 파트 세 개
		else if (d == 0) {
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
							//printf("%04x ", ProgramCounter);
							objectCodeWrite(line, locctr, 3, false, search, 1, part1, part2, part3, part4, false);
							
							//printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, part1, part2, part3);
							locctr = ProgramCounter;
						}
						else {
							objectCode[idx].ret = OPERAND_ERROR;
							errorFlag = true;
							continue;
							
						}

					}
					else if (part3[0] == 'X') {
						//int byte = toDecimal(part3[3]) + 16 * toDecimal(part3[2]);
						ProgramCounter++;
						//printf("%04x ", ProgramCounter);
						objectCodeWrite(line, locctr, 3, false, search, 1, part1, part2, part3, part4, false);

						//printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, part1, part2, part3);

						locctr = ProgramCounter;
					}
					else {
						objectCode[idx].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}

				}
				else if (!strcmp(part2, "RESB")) {
					varFlag = true;

					int size = atoi(part3);
					if (size == 0) {
						objectCode[idx].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
					
					//printf("    ");
					//printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, part1, part2, part3);
					objectCode[idx].isVariable = true;
					strcpy(objectCode[idx].label, part1);
					strcpy(objectCode[idx].opcode, part2);
					strcpy(objectCode[idx].operand_first, part3);

					ProgramCounter = locctr + size;
					locctr = ProgramCounter;
				}
				else if (!strcmp(part2, "WORD")) {
					varFlag = true;
					ProgramCounter = locctr + 3;
					//printf("%04x ", ProgramCounter);
					objectCodeWrite(line, locctr, 3, false, search, 1, part1, part2, part3, part4, false);
					//printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, part1, part2, part3);
					locctr = ProgramCounter;
					
				}
				else if (!strcmp(part2, "RESW")) {
					varFlag = true;

					int size = atoi(part3);
					if (size == 0) {
						objectCode[idx].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
					
					//printf("     ");
					//printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, part1, part2, part3);
					objectCode[idx].isVariable = true;
					strcpy(objectCode[idx].label, part1);
					strcpy(objectCode[idx].opcode, part2);
					strcpy(objectCode[idx].operand_first, part3);
					ProgramCounter = locctr + (3 * size);
					locctr = ProgramCounter;

				}
				if (varFlag) {
					
					continue;
				}
				
				memcpy(Opcode, part2, sizeof(part2));
				memcpy(Operand_first, part3, sizeof(part3));

			}
			// format 4
			if (Opcode[0] == '+') {
				isFormat4 = true;
				search = searchHashNode(Opcode + 1, (int)strlen(Opcode + 1));
			}
			else {
				search = searchHashNode(Opcode, (int)strlen(Opcode));
			}
			if (!search) {
				objectCode[idx].ret = OPERAND_NOT_FOUND_ERROR;
				errorFlag = true;
				continue;
			}
			// #########################

			// Label 없음
			ProgramCounter = locctr + (search->format + isFormat4);
			//printf("%04x ", ProgramCounter);
			if (Label[0] == '\0') {
				int operand_len = (int)strlen(part2);
				if (part2[operand_len - 1] != ',') {
					objectCode[idx].ret = OPERAND_ERROR;
					errorFlag = true;

					continue;
				}
				objectCodeWrite(line, locctr, 3, isFormat4, search, 0, part1, part2, part3, part4, false);
				//printf("%-7d%04X       %-7s%-7s%-7s\n", line, locctr, Opcode, Operand_first, Operand_second);
			}
			// Label 있음
			else {

				objectCodeWrite(line, locctr, 3, isFormat4, search, 0, part1, part2, part3, part4, true);
				//printf("%-7d%04X%-7s%-7s%-7s\n", line, locctr, Label, Opcode, Operand_first);
			}
			locctr = ProgramCounter;


		}
		// 파트 네 개
		else if (e == 0) {

			if (Label[0] != '\0') {
				memcpy(Opcode, part2, sizeof(part2));
				memcpy(Operand_first, part3, sizeof(part3));
				memcpy(Operand_second, part4, sizeof(part4));
				int operand_len = (int)strlen(Operand_first);
				if (Operand_first[operand_len - 1] != ',') {
					objectCode[idx].ret = OPERAND_ERROR;
					errorFlag = true;

					continue;
				}
			}
			else {
				objectCode[idx].ret = OPCODE_ERROR;
				errorFlag = true;
				continue;

			}
			if (Opcode[0] == '+') {
				isFormat4 = true;
				search = searchHashNode(Opcode + 1, (int)strlen(Opcode + 1));
			}
			else {
				search = searchHashNode(Opcode, (int)strlen(Opcode));
			}
			ProgramCounter = locctr + (search->format + isFormat4);
			//printf("%04x ", ProgramCounter);
			objectCodeWrite(line, locctr, 4, isFormat4, search, 0, part1, part2, part3, part4, true);
			locctr = ProgramCounter;
		}
		// 파트 다섯 개
		else {
			objectCode[idx].ret = OPCODE_ERROR;
			errorFlag = true;
			continue;
		}
	}
	assemble_end_line = line / 5;
	//printf("%04X\n", BaseAddress);
	fclose(fp);
	return errorFlag;


}


void reverseString(char str[]) {
	int size = (int)strlen(str);    // size 에 s의 크기를 저장
	char temp;                        // 문자를 뒤집을때 필요한 빈공간

	for (int i = 0; i < size / 2; i++) {
		temp = str[i];
		str[i] = str[(size - 1) - i];
		str[(size - 1) - i] = temp;
	}
}


bool pass2(char filename[MAX_PARSED_NUM + 10]) {

	
	char tmp[30];
	ProgramCounter = 0;
	bool errorFlag;
	errorFlag = false;
	Symbol* search;
	unsigned int obj;
	Symbol* base = findBinaryTreeSymbol(symbolRoot, BaseSymbol);
	if (!base) {
		objectCode[0].ret = BASE_NO_EXIST_ERROR;
		errorFlag = true;
	}
	else BaseAddress = base->loc;
	for (int i = 1; i <= assemble_end_line; i++) {
		objectCode[i].obj = 0;
		obj = 0;
		ProgramCounter = objectCode[i].loc + objectCode[i].format;
		if (objectCode[i].isVariable) {
			//printf("%-8d\t\t %-8s%-8s%-8s\n", i * 5, objectCode[i].label, objectCode[i].opcode, objectCode[i].operand_first);
			continue;
		}
		if (objectCode[i].isComment) {
			//printf("%-8d\t\t %s", i*5, objectCode[i].comment);
			
			continue;
		}
		if (objectCode[i].isDirective) {
			//printf("%-8d\t\t %-8s%-8s\n", i * 5, objectCode[i].opcode, objectCode[i].operand_first);
			continue;
		}
		if (objectCode[i].isConstant) {
			strcpy(tmp, objectCode[i].operand_first + 2);
			int len = strlen(tmp);
			tmp[len - 1] = '\0';
			if (objectCode[i].operand_first[0] == 'X') {

				bool original_zero = true;
				for (int j = 0; j < len - 1; j++) {
					if (tmp[j] != '0') {
						original_zero = false;
					}
				}
				if (original_zero == false) {
					int num = strtol(tmp, NULL, 16);
					if (num == 0) {
						objectCode[i].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
					objectCode[i].obj = obj = num;
					//printf("%-8d%04X    %-8s%-8s%-8s%-8s%X\n", i * 5, objectCode[i].loc, objectCode[i].label,
						//objectCode[i].opcode, objectCode[i].operand_first, objectCode[i].operand_second, obj);

				}
				else {
					obj = 0;
					//printf("%-8d%04X    %-8s%-8s%-8s%-8s%X\n", i * 5, objectCode[i].loc, objectCode[i].label,
						//objectCode[i].opcode, objectCode[i].operand_first, objectCode[i].operand_second, obj);
				}

				continue;
			}
			else if (objectCode[i].operand_first[0] == 'C') {
				reverseString(tmp);
				memcpy(&obj, tmp, sizeof(char)*strlen(tmp));
				objectCode[i].obj = obj;
			//	printf("%-8d%04X    %-8s%-8s%-8s%-8s%X\n", i * 5, objectCode[i].loc, objectCode[i].label, objectCode[i].opcode, objectCode[i].operand_first, objectCode[i].operand_second, obj);
			}
			continue;
		}

		switch (objectCode[i].format) {
		case 1:
			objectCode[i].obj |= objectCode[i].op;
			break;

		case 2:
			objectCode[i].obj |= (objectCode[i].op << 8);
			objectCode[i].obj |= (objectCode[i].reg1 << 4);
			objectCode[i].obj |= (objectCode[i].reg2);
			break;

		case 3: 
			/*		objectCode[i].obj |= (objectCode[i].op << 18);
					objectCode[i].obj |= (objectCode[i].nixbpe << 12);*/
			if (objectCode[i].parts == 1) {
				objectCode[i].nixbpe = 0b110000;
			}
			else if (objectCode[i].parts == 2) {
				if (objectCode[i].operand_first[0] == '@') {
					search = findBinaryTreeSymbol(symbolRoot, objectCode[i].operand_first + 1);
					if (!search) {
						errorFlag = true;
						objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
						continue;
					}
					int target = search->loc;
					int range = target - ProgramCounter;
					if (range >= -2048 || range <= 2047) {
						objectCode[i].nixbpe = 0b100010;
						objectCode[i].disp = range;
					}
					else {
						range = target - BaseAddress;
						if (range < 0 || range>4095) {
							objectCode[i].ret = TOO_FAR_ERROR;
							errorFlag = true;
							continue;
						}
						objectCode[i].nixbpe = 0b100100;
						objectCode[i].disp = range;
					}
				}
				else if (objectCode[i].operand_first[0] == '#') {
					if (!strcmp(objectCode[i].operand_first + 1, "0")) {
						objectCode[i].nixbpe = 0b010000;
						objectCode[i].disp = 0;
					}
					else {
						search = findBinaryTreeSymbol(symbolRoot, objectCode[i].operand_first + 1);
						if (!search) {
							int num = atoi(objectCode[i].operand_first + 1);
							if (!num) {
								errorFlag = true;
								objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
								continue;
							}
							else {
								
								objectCode[i].nixbpe = 0b010000;
								objectCode[i].disp = num;
							}
						}
						else {
							int range = search->loc - ProgramCounter;
							if (range >= -2048 && range <= 2047) {
								objectCode[i].nixbpe = 0b010010;
								objectCode[i].disp = range;
							}
							else {
								range = search->loc - BaseAddress;
								if (range < 0 || range>4095) {
									objectCode[i].ret = TOO_FAR_ERROR;
									errorFlag = true;
									continue;
								}
								objectCode[i].nixbpe = 0b010100;
								objectCode[i].disp = range;
							}
						}

					}
				}
				else {
					search = findBinaryTreeSymbol(symbolRoot, objectCode[i].operand_first);
					if (!search) {
						objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
						errorFlag = true;
						continue;
					}
					int range = search->loc - ProgramCounter;
					if (range >= -2048 && range <= 2047) {
						objectCode[i].nixbpe = 0b110010;
						objectCode[i].disp = range;
					}
					else {
						range = search->loc - BaseAddress;
						if (range < 0 || range>4095) {
							objectCode[i].ret = TOO_FAR_ERROR;
							errorFlag = true;
							continue;
						}
						objectCode[i].nixbpe = 0b110100;
						objectCode[i].disp = range;
					}

				}
			}
			// STCH BUFFER, X
			else if (objectCode[i].parts == 3) {
				strcpy(tmp, objectCode[i].operand_first);
				int len = strlen(tmp);
				if (tmp[len - 1] == ',') {
					tmp[len - 1] = '\0';
					Symbol* search = findBinaryTreeSymbol(symbolRoot, tmp);
					if (!search) {
						objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
						errorFlag = true;
						continue;
					}
					if (!strcmp(objectCode[i].operand_second, "X")) {
						int range = search->loc - ProgramCounter;
						if (range >= -2048 && range <= 2047) {
							objectCode[i].nixbpe = 0b111010;
							objectCode[i].disp = range;
						}
						else {
							range = search->loc - BaseAddress;
							if (range < 0 || range>4095) {
								objectCode[i].ret = TOO_FAR_ERROR;
								errorFlag = true;
								continue;
							}
							objectCode[i].nixbpe = 0b111100;
							objectCode[i].disp = range;
						}
					}
					else {
						objectCode[i].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
				}
				else {
					objectCode[i].ret = OPERAND_ERROR;
					errorFlag = true;
					continue;
				}

			}
			else if (objectCode[i].parts == 4) {
				
			}
			//if()
			break;
		

		case 4: 
			if (objectCode[i].operand_first[0] == '@') {
				search = findBinaryTreeSymbol(symbolRoot, objectCode[i].operand_first + 1);
				if (!search) {
					errorFlag = true;
					objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
					continue;
				}
				int target = search->loc;
				int range = target - ProgramCounter;
				if (range >= -2048 || range <= 2047) {
					objectCode[i].nixbpe = 0b100011;
					objectCode[i].addr = range;
				}
				else {
					range = target - BaseAddress;
					if (range < 0 || range>4095) {
						objectCode[i].ret = TOO_FAR_ERROR;
						errorFlag = true;
						continue;
					}
					objectCode[i].nixbpe = 0b100101;
					objectCode[i].addr = range;
				}
			}
			else if (objectCode[i].operand_first[0] == '#') {
				if (!strcmp(objectCode[i].operand_first + 1, "0")) {
					objectCode[i].nixbpe = 0b010001;
					objectCode[i].addr = 0;
				}
				else {
					search = findBinaryTreeSymbol(symbolRoot, objectCode[i].operand_first + 1);
					if (!search) {
						int num = atoi(objectCode[i].operand_first + 1);
						if (!num) {
							errorFlag = true;
							objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
							continue;
						}
						else {

							objectCode[i].nixbpe = 0b010001;
							objectCode[i].addr = num;
						}
					}
					else {
						
						objectCode[i].nixbpe = 0b110001;
						objectCode[i].addr = search->loc;
						
					}

				}
			}
			else {
				search = findBinaryTreeSymbol(symbolRoot, objectCode[i].operand_first);
				if (!search) {
					objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
					errorFlag = true;
					continue;
				}
				
				objectCode[i].nixbpe = 0b110001;
				objectCode[i].addr = search->loc;
				

			}

			break;
		
		}
		unsigned int obj = 0;
		switch (objectCode[i].format) {
		case 1:
			obj |= (objectCode[i].op);
			break;

		case 2:
			obj |= (objectCode[i].op << 8);
			obj |= (objectCode[i].reg1 << 4);
			obj |= objectCode[i].reg2;
			break;

		case 3:
			obj |= (objectCode[i].op << 16);
			obj |= (objectCode[i].nixbpe << 12);
			obj |= (objectCode[i].disp);
			break;

		case 4:
			obj |= (objectCode[i].op << 24);
			obj |= (objectCode[i].nixbpe << 20);
			obj |= (objectCode[i].addr);
			break;
		}
		objectCode[i].obj = obj;
		//printf("%-8d%04X    %-8s%-8s%-8s%-8s%X\n", i * 5, objectCode[i].loc, objectCode[i].label, objectCode[i].opcode, objectCode[i].operand_first, objectCode[i].operand_second, obj);

	}
	return errorFlag;
}


void showPassError() {
	if (objectCode[0].ret == BASE_NO_EXIST_ERROR) {
		printf("Program has no base!\n");
	}
	for (int i = 1; i <= assemble_end_line; i++) {
		if (objectCode[i].ret != NORMAL) {
			printf("Line %d : ", i * 5);
			switch (objectCode[i].ret) {
			case OPCODE_ERROR:
				printf("Opcode error! ");
				break;

			case OPERAND_ERROR:
				printf("Operand error! ");
				break;

			case DUPLICATE_SYMBOL_ERROR:
				printf("Symbol Duplicated error! ");
				break;
			case TOO_FAR_ERROR:
				printf("Too Far to jump! ");
				break;
			case OPERAND_NOT_FOUND_ERROR:
				printf("Symbol not exist ");
				break;
			default:
				printf("Other error! ");
			}
			printf("%s %s %s %s\n", objectCode[i].label, objectCode[i].opcode, 
				objectCode[i].operand_first, objectCode[i].operand_second);
		}
	}
}



void writeFiles(char lstFile[], char objFile[]) {
	FILE* fp = fopen(lstFile, "w");
	if (fp == NULL) {
		FILE_ERROR();
		return;
	}
	for (int i = 1; i <= assemble_end_line; i++) {
		int line = i * 5;
		Formats* forShow = &objectCode[i];
		
		if (forShow->isComment) {
			fprintf(fp, "%-8d                %s", line, forShow->comment);
			printf("%-8d                %s", line, forShow->comment);
		}
		else if (forShow->isConstant) {
			fprintf(fp, "%-8d%04X    %-8s%-8s%-8s%-8s", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);
			printf("%-8d%04X    %-8s%-8s%-8s%-8s", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);
			if (forShow->operand_first[0] == 'X') {
				char tmp[30];
				strcpy(tmp, forShow->operand_first + 2);
				tmp[strlen(tmp) - 1] = '\0';
				fprintf(fp, "%-s\n", tmp);
				printf("%-s\n", tmp);

			}
			else {
				fprintf(fp, "%X\n", forShow->obj);
				printf("%X\n", forShow->obj);
			}
		}
		else if (forShow->isVariable) {
			fprintf(fp, "%-8d%04X    %-8s%-8s%-8s%-8s\n", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);
			printf("%-8d%04X    %-8s%-8s%-8s%-8s\n", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);

		}
		else if (forShow->isDirective) {
			fprintf(fp, "%-8d        %-8s%-8s%-8s%-8s\n", line, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);
			printf("%-8d        %-8s%-8s%-8s%-8s\n", line, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);

		}
		else {
			fprintf(fp, "%-8d%04X    %-8s%-8s%-8s%-8s", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);
			printf("%-8d%04X    %-8s%-8s%-8s%-8s", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);

			switch (forShow->format) {
			case 1:
				fprintf(fp, "%02X\n", forShow->obj);
				printf("%02X\n", forShow->obj);
				break;

			case 2:
				fprintf(fp, "%04X\n", forShow->obj);
				printf("%04X\n", forShow->obj);				
				break;

			case 3:
				fprintf(fp, "%06X\n", forShow->obj);
				printf("%06X\n", forShow->obj);
				break;

			case 4:
				fprintf(fp, "%08X\n", forShow->obj);
				printf("%08X\n", forShow->obj);
				break;
				
			default:
				printf("\n");
				break;
			}
		}
	}
	fclose(fp);
}
void doAssemble(char parsedInstruction[][MAX_PARSED_NUM + 10]) {


	char temp[] = "lst";
	char temp2[] = "obj";
	strcpy(lstFile, parsedInstruction[1]);
	strcpy(objFile, parsedInstruction[1]);
	int len = (int)strlen(lstFile);
	objFile[len - 3] = '\0';
	lstFile[len - 3] = '\0';
	strcat(lstFile, temp);
	strcat(objFile, temp2);

	symbolRoot = getNewSymbolNode();

	memset(objectCode, 0, sizeof(objectCode));
	if (pass1(parsedInstruction[1]) == false) {
		if (pass2(parsedInstruction[1]) == false) {
			printf("Successfullly %s %s\n", parsedInstruction[0], parsedInstruction[1]);
			writeFiles(lstFile, objFile);
		}
		else {
			showPassError();

		}
	}
	else {
		showPassError();
	}



}