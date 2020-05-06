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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/*																				  //
 *								Binary Search Tree에 관한 함수들					  //
 *																				  //
 * getNewSymbolNode(), findBinaryTreeSymbol, insertBinaryTreeSymbol()			  */
 // 새로운 Tree 노드를 하나 생성하는 함수.
Symbol* getNewSymbolNode() {
	Symbol* temp = (Symbol*)malloc(sizeof(Symbol));
	temp->symbol[0] = '\0';
	temp->loc = -1;
	temp->left = temp->right = NULL;
	return temp;
}

// root에 있는 symbol과 찾고자 하는 symbol을 비교하여 탐색.
Symbol* findBinaryTreeSymbol(Symbol* root, char symbol[]) {
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
bool insertBinaryTreeSymbol(Symbol* root, char symbol[], int loc) {
	// 빈 노드이면 여기에다가 넣고, true를 반환한다.
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
	// symbol이 작으면 root->left로 들어가서 넣음.
	if (res > 0) {
		if (!root->left) {
			Symbol* temp = getNewSymbolNode();
			root->left = temp;
		}
		return insertBinaryTreeSymbol(root->left, symbol, loc);
	}
	// symbol이 크면 root->right로 들어가서 넣음.
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

//   						End of Binary Search Tree Function					  //
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/* 
	들어온 string을 가지고 단순 비교하여 register를 찾는다.
	문자열 길이가 길지 않아 단순 비교 하였다.
*/
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

/*
	void objectCodeWrite()

인자:
	int line : line 넘버.
	int loc : 현재 넘어온 location counter
	int parts : 공백으로 파싱된 문자열의 개수
	bool isFormat4 : format4인지 나타내는 boolean
	OpNode* serach : opcode에 해당하는 OpNode가 넘어옴.
	int constatnt : 상수(BYTE, WORD)이면 1, 아니면 0
	char part1[], part2[], part3[], part4[] : 파싱되어 넘어온 문자열들
	bool lableFlag : Label이 있으면 true, 없으면 false

기능 : 
	위 넘어온 인자들의 정보를 바탕으로 intermediate File에 넣지 않고 intermediate data structure에 저장한다.
	굳이 파일로 저장하지 않은 이유는 중간 파일에 저장하면 다시 읽어들일 때 파싱하는 overhead가 있기 때문이다.
*/
void objectCodeWrite(int line, int loc, int parts, bool isFormat4, OpNode* search, int constant, char part1[], char part2[], char part3[], char part4[], bool labelFlag) {
	int l = line / 5;

	// 기본적으로 search->format에 저장된 수는 1, 2, 3이다. 따라서 format 4라면 isFormat4를 더해주어야 한다.
	objectCode[l].format = search->format + isFormat4;
	objectCode[l].loc = loc;
	objectCode[l].op = search->value;
	
	int format = objectCode[l].format;

	// 상수 (BYTE or WORD)라면 따로 처리할 것 없이 문자열만 복사하고 함수를 종료한다.
	if (constant) {
		strcpy(objectCode[l].label, part1);
		strcpy(objectCode[l].opcode, part2);
		strcpy(objectCode[l].operand_first, part3);
		objectCode[l].isConstant = true;
		return;
	}
	
	// Label이 있는 경우
	if (labelFlag == true) {
		strcpy(objectCode[l].label, part1);
		strcpy(objectCode[l].opcode, part2);
		strcpy(objectCode[l].operand_first, part3);
		strcpy(objectCode[l].operand_second, part4);
		// Label이 있기 때문에 실제로 Object code를 생성할 때 필요한 부분은 parts - 1이다.
		objectCode[l].parts = parts - 1;

		// Label Opcode 형태
		if (parts == 2) {
			// format 2라면 opcode(8), reg1(4), reg2(4)
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

			// format 3이라면 opcode(6) nixbpe(6) disp(12)
			else if (format == 3) {
				// indirect addressing
				if (part2[0] == '@') {
					objectCode[l].nixbpe = 0b100000;
				}
				// immediate addressing
				else if (part2[0] == '#') {
					objectCode[l].nixbpe = 0b010000;
				}
				// simple addressing
				else {
					objectCode[l].nixbpe = 0b110000;
				}

			}
		}

		// Label Opcode Operand 형태
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
			else if (format == 3 || format == 4) {
				// indirect addressing
				if (part2[0] == '@') {
					objectCode[l].nixbpe = 0b100000;
				}
				// immediate addressing
				else if (part2[0] == '#') {
					objectCode[l].nixbpe = 0b010000;
				}
				// simple addressing
				else {
					objectCode[l].nixbpe = 0b110000;
				}
			}
		}
		// parts가 4개일 때
		else if (parts == 4) {
			// 마지막 파트가 "X"이면 X reg를 사용함
			if (!strcmp(part4, "X")) {
				objectCode[l].nixbpe = 0b111000;
			}
		}
		// parts가 5개 이상이면 오류
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

		// parts가 1개이면 RSUB와 같은 명령어로, 따로 처리할 사항이 없다.
		if (parts == 1) {
			;
		}

		// Label이 있는 경우와 동일한 구조.
		else if (parts == 2) {
			if (format == 2) {
				REG num = findRegNumber(part2);
				if (num != -1) {
					objectCode[l].reg1 = num;
				}
				else objectCode[l].ret = OPERAND_ERROR;
			}
			else if (format == 3) {
				// indirect addressing
				if (part2[0] == '@') {
					objectCode[l].nixbpe = 0b100000;
				}
				// immediate addressing
				else if (part2[0] == '#') {
					objectCode[l].nixbpe = 0b010000;
				}
				else {
					objectCode[l].nixbpe = 0b110000;
				}
			}
			else if (format == 4) {
				objectCode[l].nixbpe = 0b110001;
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
				if (!strcmp(part3, "X")) {
					objectCode[l].nixbpe = 0b111000;
				}	

			}
			else if (format == 4) {
				objectCode[l].nixbpe = 0b110001;
			}
		}
		else {
			objectCode[l].ret = OPERAND_ERROR;
		}
	}
}

// Symbol을 나타내는 Binary Search Tree를 모두 순회하며 해제한다.
void deleteSymbolTree(Symbol* root) {
	if (root) {
		deleteSymbolTree(root->left);
		deleteSymbolTree(root->right);
		free(root);
	}
}

/*
	Assembler의 PASS1을 담당.
인자:
	filename : 2_5.asm과 같이 .asm파일이 넘어온다.
*/
bool pass1(char filename[]) {
	// 문자열을 처리하기 위해 지역 변수로 char 배열들을 선언하였다.
	char lines[100];
	char part1[30], part2[30], part3[30], part4[30], part5[30];
	char Label[30], Opcode[30], Operand_first[30], Operand_second[30];
	char a, b, c, d, e;

	// starting_address는 START DIRECTIVE로 나타나는 시작 주소를 저장하기 위함.
	// locctr은 location counter를 나타냄
	int starting_address = 0, locctr = 0;
	int line = 0, idx;
	OpNode* search;

	//Formats binaryCode
	FILE* fp = fopen(filename, "r");

	memset(BaseSymbol, 0, sizeof(BaseSymbol));
	// assemble하기에 앞서 일단 binary search tree에 저장된 내용을 지우고 새로이 할당한다.
	deleteSymbolTree(symbolRoot);
	symbolRoot = getNewSymbolNode();
	
	// START가 나올 때 start_flag가 세팅된다.
	bool start_flag = false;

	// 오류가 하나라도 있으면 errorFlag가 세팅된다.
	bool errorFlag = false;
	
	while (1) {
		bool isFormat4 = false;

		// 한 라인을 입력받기 전에 lines를 초기화한다.
		lines[0] = 0;
		fgets(lines, 100, fp);
	
		Label[0] = '\0';
		Opcode[0] = '\0';
		Operand_first[0] = '\0';
		Operand_first[0] = '\0';
		if (lines[0] == '\0') {
			break;
		}
		
		// line은 5씩 증가
		line += 5;
		// a, b, c, d, e와 part1, part2, part3, part4, part5는 한 줄을 읽을 때 공백으로 나누어진 part가 몇 개인지 구분하기 위해 전부 '\0'과 EOS로 초기화한다.
		a = b = c = d = e = 0;
		part1[0] = part2[0] = part3[0] = part4[0] = part5[0] = '\0';

		idx = line / 5;
		// sscanf를 이용하여 다음과 같은 형식으로 입력받는다. 
		// 만약 그냥 빈 줄을 입력 받는다면 part1[0]은 '\0'이고 그 뒤에 나오는 변수들도 모두 아무 것도 입력 받지 못한다.
		sscanf(lines, "%s%c%s%c%s%c%s%c%s%c", part1, &a, part2, &b, part3, &c, part4, &d, part5, &e);

		// START가 있어야 시작한다.
		if (!start_flag) {
			if (!strcmp(part2, "START")) {
				// START가 나오면 part3에 시작 주소가 나올 것이다. 이것이 WRONG_HEXA이면 안된다.
				for (int i = (int)strlen(part3) - 1; i >= 0; i--) {
					int next = toDecimal(part3[i]);
					if (next == WRONG_HEXA) {
						objectCode[idx].ret = ADDRESS_INPUT_ERROR;
						errorFlag = true;
						continue;
					}
				}

				// errorFlag가 세팅되면 파싱할 ㅏ필요가 없음.
				if (errorFlag) continue;

				// assemble_start_line에 시작 index를 담는다.
				assemble_start_line = idx;

				// part3를 strtol을 이용하여 16진수로 변환
				
				int len = strlen(part3);
				bool original_zero = true;
				for (int j = 0; j < len; j++) {
					if (part3[j] != '0') {
						original_zero = false;
					}
				}
				// 원래 zero가 아닌데
				if (original_zero == false) {
					int num = strtol(part3, NULL, 16);
					// num이 0이면 잘못된 거임
					if (num == 0) {
						objectCode[idx].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
					starting_address = num;
				}
				// 원래 zero
				else {
					starting_address = 0;
				}

				start_flag = true;
				locctr = starting_address;
				objectCode[idx].ret = NORMAL;

				// start면 format을 0으로 설정
				objectCode[idx].format = 0;

				objectCode[idx].loc = starting_address;
				strcpy(objectCode[idx].label, part1);
				strcpy(objectCode[idx].opcode, part2);
				strcpy(objectCode[idx].operand_first, part3);
				continue;
			}
			else {
				// START가 나오기 전에 comment가 아닌 어떤 문자가 나오면 오류
				if (part1[0] == '.') {
					objectCode[idx].isComment = true;
					objectCode[idx].comment = (char*)malloc(sizeof(char) * 200);
					strcpy(objectCode[idx].comment, lines);
				}
				else if (part1[0] != '\0') {
					objectCode[idx].ret = OPCODE_ERROR;
				}
				else if (part1[0] == '\0') {
					objectCode[idx].isComment = true;
					objectCode[idx].comment = (char*)malloc(sizeof(char) * 2);
					objectCode[idx].comment[0] = '\n';
					objectCode[idx].comment[1] = '\0';
				}
				continue;
			}
		}

		// 일단 문장을 NORMAL이라고 생각함.
		objectCode[idx].ret = NORMAL;

		// a가 EOS라는 것은 이라는 것은 공백만 들어왔다는 것.
		if (a == '\0') {
			objectCode[idx].isComment = true;
			objectCode[idx].comment = (char*)malloc(sizeof(char) * 2);
			objectCode[idx].comment[0] = '\n';
			objectCode[idx].comment[1] = '\0';
			continue;
		}

		// part1의 첫 번째 문자가 '.'이라는 것은 comment line임.
		if (part1[0] == '.') {
			objectCode[idx].isComment = true;
			objectCode[idx].comment = (char*)malloc(sizeof(char) * 200);
			strcpy(objectCode[idx].comment, lines);
			continue;
		}

		// part1의 첫 번째 문자가 EOS라는 것은 공백인 줄이 들어왔다는 것.
		else if (part1[0] == '\0') {
			objectCode[idx].isComment = true;
			objectCode[idx].comment = (char*)malloc(sizeof(char) * 2);
			objectCode[idx].comment[0] = '\0';
			objectCode[idx].comment[1] = '\0';
			continue;
		}
		// part1이 "END"이면 일단 Directive라는 것을 표시하고 종료(break)
		if (!strcmp(part1, "END")) {
			objectCode[idx].loc = locctr;
			objectCode[idx].isDirective = true;
			strcpy(objectCode[idx].opcode, part1);
			strcpy(objectCode[idx].operand_first, part2);
			break;
		}

		// BASE이면 Directive임을 표시하고 넘어감.
		if (!strcmp(part1, "BASE")) {
			strcpy(BaseSymbol, part2);
			strcpy(objectCode[idx].opcode, part1);
			strcpy(objectCode[idx].operand_first, part2);
			objectCode[idx].isDirective = true;
			continue;
		}
		
		// Label 있을 때
		if (lines[0] != ' ') {
			// 이미 Label이 테이블에 있으면 에러
			if (!insertBinaryTreeSymbol(symbolRoot, part1, locctr)) {
				objectCode[idx].ret = DUPLICATE_SYMBOL_ERROR;
				errorFlag = true;
				continue;
			}
			memcpy(Label, part1, sizeof(part1));
		}


		// 파트 한 개
		if (b == 0) {
			// 파트가 하나인데 Label이면 오류
			if (Label[0] != '\0') {
				objectCode[idx].ret = OPCODE_ERROR;
				errorFlag = true;
				continue;
			}
			// Label이 아니면 opcode이므로 HashTable에서 찾는다.
			search = searchHashNode(part1, (int)strlen(part1));
			// 못 찾으면 오류.
			if (!search) {
				objectCode[idx].ret = OPERAND_NOT_FOUND_ERROR;
				errorFlag = true;
				continue;
			}
			
			ProgramCounter = locctr + search->format;
			// objectCodeWrite를 통해 중간 자료구조인 (objectCode 구조체 배열)에 삽입한다.
			objectCodeWrite(line, locctr, 1, isFormat4, search, 0, part1, part2, part3, part4, false);
			
			locctr = ProgramCounter;
		}

		// 파트 두 개
		else if (c == 0) {
			// Label이 아니면
			if (Label[0] == '\0') {
				memcpy(Opcode, part1, sizeof(part1));
				memcpy(Operand_first, part2, sizeof(part2));
			}
			// Label이면
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
			}
			// format 4가 아님
			else {
				search = searchHashNode(Opcode, (int)strlen(Opcode));
				if (!search) {
					objectCode[idx].ret = OPERAND_NOT_FOUND_ERROR;
					errorFlag = true;
					continue;
					
				}
			}
			ProgramCounter = (locctr + search->format + isFormat4);
			// Label이 없을 때
			if (Label[0] == '\0') {
				objectCodeWrite(line, locctr, 2, isFormat4, search, 0, part1, part2, part3, part4, false);
			}
			//Label이 있을 때.
			else {
				objectCodeWrite(line, locctr, 2, isFormat4, search, 0, part1, part2, part3, part4, true);
			}
			locctr = ProgramCounter;
		}

		// 파트 세 개
		else if (d == 0) {
			// Label 없을 때
			if (Label[0] == '\0') {
				memcpy(Opcode, part1, sizeof(part1));
				memcpy(Operand_first, part2, sizeof(part2));
				memcpy(Operand_second, part3, sizeof(part3));

			}
			// Label이 있는 위치에 뭔가 있을 때.
			else {
				// variable이나 constant인지 확인하자.
				bool varFlag = false;
				// BYTE일 때.
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
						char tmp[30];
						strcpy(tmp, part3 + 2);
						int len = strlen(tmp);
						tmp[len - 1] = '\0';
						ProgramCounter += (len - 1) / 2;
						
						objectCodeWrite(line, locctr, 3, false, search, 1, part1, part2, part3, part4, false);

						locctr = ProgramCounter;
					}
					else {
						objectCode[idx].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
				}
				// RESB인 BYTE 변수일 때
				else if (!strcmp(part2, "RESB")) {
					varFlag = true;
					int size = atoi(part3);
					if (size == 0) {
						objectCode[idx].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
					
					objectCode[idx].isVariable = true;
					objectCode[idx].loc = locctr;

					strcpy(objectCode[idx].label, part1);
					strcpy(objectCode[idx].opcode, part2);
					strcpy(objectCode[idx].operand_first, part3);

					ProgramCounter = locctr + size;
					locctr = ProgramCounter;
				}

				// WORD일 때
				else if (!strcmp(part2, "WORD")) {
					varFlag = true;
					ProgramCounter = locctr + 3;
					objectCodeWrite(line, locctr, 3, false, search, 1, part1, part2, part3, part4, false);
					locctr = ProgramCounter;		
				}

				// RESW인 WORD 변수일 때.
				else if (!strcmp(part2, "RESW")) {
					varFlag = true;
					// part3에 있는 10진수의 수만큼을 WORD 변수로 생성한다.
					int size = atoi(part3);
					if (size == 0) {
						objectCode[idx].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
					
					objectCode[idx].isVariable = true;
					objectCode[idx].loc = locctr;

					strcpy(objectCode[idx].label, part1);
					strcpy(objectCode[idx].opcode, part2);
					strcpy(objectCode[idx].operand_first, part3);

					// WORD의 크기가 3bytes이므로 3*size만큼 증가해야 한다.
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
			// format 4가 아닐 때
			else {
				search = searchHashNode(Opcode, (int)strlen(Opcode));
			}
			// opcode를 HashTable에서 못 찾으면 오류
			if (!search) {
				objectCode[idx].ret = OPERAND_NOT_FOUND_ERROR;
				errorFlag = true;
				continue;
			}

			ProgramCounter = locctr + (search->format + isFormat4);
			
			// Label 없을 때
			if (Label[0] == '\0') {
				int operand_len = (int)strlen(part2);
				// part2의 마지막에 ','가 있어야 한다.
				if (part2[operand_len - 1] != ',') {
					objectCode[idx].ret = OPERAND_ERROR;
					errorFlag = true;
					continue;
				}
				objectCodeWrite(line, locctr, 3, isFormat4, search, 0, part1, part2, part3, part4, false);
			}

			// Label 있음
			else {
				objectCodeWrite(line, locctr, 3, isFormat4, search, 0, part1, part2, part3, part4, true);
			}
			locctr = ProgramCounter;
		}

		// 파트 네 개
		else if (e == 0) {
			// Lable이 있을 때
			if (Label[0] != '\0') {
				memcpy(Opcode, part2, sizeof(part2));
				memcpy(Operand_first, part3, sizeof(part3));
				memcpy(Operand_second, part4, sizeof(part4));
				int operand_len = (int)strlen(Operand_first);
				// operand_first의 끝에 ','가 있어야 한다.
				if (Operand_first[operand_len - 1] != ',') {
					objectCode[idx].ret = OPERAND_ERROR;
					errorFlag = true;
					continue;
				}
			}
			// Label 없으면 무조건 에러.
			// 이유는 opcode operand_first, operand_second, operand_third는 이 머신에서 존재하지 않음.
			else {
				objectCode[idx].ret = OPCODE_ERROR;
				errorFlag = true;
				continue;
			}

			// format 4
			if (Opcode[0] == '+') {
				isFormat4 = true;
				search = searchHashNode(Opcode + 1, (int)strlen(Opcode + 1));
			}
			// format 4가 아님
			else {
				search = searchHashNode(Opcode, (int)strlen(Opcode));
			}
			// search가 없으면 
			if (!search) {
				objectCode[idx].ret = OPERAND_NOT_FOUND_ERROR;
				errorFlag = true;
				continue;
			}
			objectCodeWrite(line, locctr, 4, isFormat4, search, 0, part1, part2, part3, part4, true);
			locctr = ProgramCounter;
		}

		// 파트 다섯 개 이상은 무조건 에러.
		else {
			objectCode[idx].ret = OPCODE_ERROR;
			errorFlag = true;
			continue;
		}
	}

	assemble_end_line = line / 5;
	fclose(fp);
	return errorFlag;
}

// string을 뒤집어 주는 함수.
// C언어는 Little Endian이기 때문에 Big Endian을 사용하는 SIC/XE 머신을 위해 문자열을 뒤집어 주어야 할 때가 있음.
void reverseString(char str[]) {
	int size = (int)strlen(str);    // size 에 s의 크기를 저장
	char temp;                      // 문자를 뒤집을때 필요한 빈 공간

	for (int i = 0; i < size / 2; i++) {
		temp = str[i];
		str[i] = str[(size - 1) - i];
		str[(size - 1) - i] = temp;
	}
}


/*
	PASS2를 담당하는 함수.
인자 : 없음

기능 : pass1을 통해 중간 자료구조 (objectCode 구조체 배열)에 저장된 데이터를 가지고 object code를 생성한다.
*/
bool pass2() {
	char tmp[30];
	bool errorFlag = false;
	Symbol* search;
	unsigned int obj;
	ProgramCounter = 0;
	
	// pass2를 시작하기에 앞서 base를 찾아야 한다. pass1에서 "BASE"를 찾으면 그 뒤에 것을 base로 두기로 했다.
	Symbol* base = findBinaryTreeSymbol(symbolRoot, BaseSymbol);
	// base 못 찾으면 에러.
	if (BaseSymbol[0] == '\0' || !base) {
		objectCode[0].ret = BASE_NO_EXIST_ERROR;
		errorFlag = true;
	}
	// 찾았으면 BaseAddress 에 base->loc를 담는다.
	else BaseAddress = base->loc;

	// 첫 번째 줄부터 마지막 줄까지 전부 처리해야 한다.
	for (int i = 1; i <= assemble_end_line; i++) {
		objectCode[i].obj = 0;

		// int obj는 디버깅을 위해 두었음.
		obj = 0;
		
		ProgramCounter = objectCode[i].loc + objectCode[i].format;

		// Variable이나 Comment나 Directive는 Object Code를 생성하지 않음.
		if (objectCode[i].isVariable || objectCode[i].isComment || objectCode[i].isDirective) {
			continue;
		}

		// 상수일 때(BYTE or WORD)
		if (objectCode[i].isConstant) {
			strcpy(tmp, objectCode[i].operand_first + 2);
			int len = strlen(tmp);
			tmp[len - 1] = '\0';

			// BYTE일 때,
			if (!strcmp(objectCode[i].opcode, "BYTE")) {
				// 첫 번째 글자가 'X'이면
				if (objectCode[i].operand_first[0] == 'X') {

					// 따옴표로 묶인 문자들이 원래 zero인지 먼저 판별해야 함.
					bool original_zero = true;
					for (int j = 0; j < len - 1; j++) {
						if (tmp[j] != '0') {
							original_zero = false;
						}
					}
					// 원래 zero가 아니면 strtol로 분해해봄.
					if (original_zero == false) {
						int num = strtol(tmp, NULL, 16);
						// num이 0이면 잘못된 값이 반환된 것임.
						if (num == 0) {
							objectCode[i].ret = OPERAND_ERROR;
							errorFlag = true;
							continue;
						}
						objectCode[i].obj = obj = num;
					}
					// zero가 맞으면 그냥 zero로 냅두면 됨.
					else {
						objectCode[i].obj = obj = 0;
					}
					continue;
				}
				// C로 시작하면 character임.
				else if (objectCode[i].operand_first[0] == 'C') {
					// 따옴표로 묶인 문자열이 tmp이므로 이를 뒤집어서 저장한다.
					// Big Endian으로 저장해야 하기 때문이다.
					reverseString(tmp);
					memcpy(&obj, tmp, sizeof(char) * strlen(tmp));
					objectCode[i].obj = obj;
				}
			}

			// WORD일 때
			else if (!strcmp(objectCode[i].opcode, "WORD")) {
				// 원래 zero인지 판단. 
				bool original_zero = true;
				for (int j = 0; j < len - 1; j++) {
					if (objectCode[i].operand_first[j] != '0') {
						original_zero = false;
					}
				}
				if (original_zero == false) {
					int num = atoi(objectCode[i].operand_first);
					if (num == 0) {
						objectCode[i].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
					objectCode[i].obj = obj = num;
				}
				else {
					objectCode[i].obj = obj = 0;
				}
			}
			continue;
		}

		// objectCode[i]의 format에 따라 ObjectCode가 결정된다.
		// 이미 우리는 PASS1에서 objectCode의 op field, reg field는 결정하였다.
		switch (objectCode[i].format) {

		case 3: 
			// format 3인데 parts가 1개이면 RSUB와 같은 것이다. 이는 Simple Addressing으로 결정된다.
			if (objectCode[i].parts == 1) {
				objectCode[i].nixbpe = 0b110000;
			}
			// format 2인데 parts3가 2개일 때
			else if (objectCode[i].parts == 2) {
				// Indirect addressing
				if (objectCode[i].operand_first[0] == '@') {
					search = findBinaryTreeSymbol(symbolRoot, objectCode[i].operand_first + 1);
					if (!search) {
						errorFlag = true;
						objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
						continue;
					}

					// 일단 PC relative로 도전해본다.
					int target = search->loc;
					int range = target - ProgramCounter;
					if (range >= -2048 || range <= 2047) {
						objectCode[i].nixbpe = 0b100010;
						objectCode[i].disp = range;
					}
					// PC로 안되면 BASE로 도전해본다.
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

				// Immediate addressing이다.
				else if (objectCode[i].operand_first[0] == '#') {
					// 뒤에 나오는 게 0이면 그냥 바로 disp field에 0을 삽입.
					if (!strcmp(objectCode[i].operand_first + 1, "0")) {
						objectCode[i].nixbpe = 0b010000;
						objectCode[i].disp = 0;
					}
					// 0이 아니면
					else {
						// 일단 Symbol에서 찾아본다.
						search = findBinaryTreeSymbol(symbolRoot, objectCode[i].operand_first + 1);
						// 없으면 바로 에러나는 것이 아니다. 숫자일 수도 있기 때문이다.
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
						// Symbol에 있으면
						else {
							// PC relative로 도전.
							int range = search->loc - ProgramCounter;
							if (range >= -2048 && range <= 2047) {
								objectCode[i].nixbpe = 0b010010;
								objectCode[i].disp = range;
							}
							// PC가 안되면 BASE relative로 도전.
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
				// Indirect, Immediate addressing이 둘 다 아니면 Simple addressing
				else {
					// Symbol을 찾고, 없으면 에러.
					search = findBinaryTreeSymbol(symbolRoot, objectCode[i].operand_first);
					if (!search) {
						objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
						errorFlag = true;
						continue;
					}

					// 마찬가지로 PC relative로 먼저 도전.
					int range = search->loc - ProgramCounter;
					if (range >= -2048 && range <= 2047) {
						objectCode[i].nixbpe = 0b110010;
						objectCode[i].disp = range;
					}
					// 안되면 BASE relative로 도전.
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

			// eg) STCH BUFFER, X 와 같은 형태
			else if (objectCode[i].parts == 3) {

				// operand_first에서 ','를 제거하고 처리하기 위함.
				strcpy(tmp, objectCode[i].operand_first);
				int len = strlen(tmp);

				// ','가 operand_first에 있어야 함.
				if (tmp[len - 1] == ',') {
					tmp[len - 1] = '\0';
					search = findBinaryTreeSymbol(symbolRoot, tmp);
					if (!search) {
						objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
						errorFlag = true;
						continue;
					}

					// 마지막 파트가 "X"이어야 함.
					if (!strcmp(objectCode[i].operand_second, "X")) {
						// 마찬가지로 PC relative로 도전해보고 안되면 BASE relative로 바꾼다.
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
					// "X"가 아니면 주어진 환경에서는 일단 에러로 처리함.
					else {
						objectCode[i].ret = OPERAND_ERROR;
						errorFlag = true;
						continue;
					}
				}

				// ','가 없으면 OPERAND_ERROR
				else {
					objectCode[i].ret = OPERAND_ERROR;
					errorFlag = true;
					continue;
				}
			}
			break;
		
		// format 4
		case 4: 
			// Indirect Addressing
			if (objectCode[i].operand_first[0] == '@') {
				search = findBinaryTreeSymbol(symbolRoot, objectCode[i].operand_first + 1);
				if (!search) {
					errorFlag = true;
					objectCode[i].ret = OPERAND_NOT_FOUND_ERROR;
					continue;
				}
				// 찾았으면 nixbpe에 0b100001을 세팅하고, addr field는 search->loc를 그대로 넣는다.
				objectCode[i].nixbpe = 0b100001;
				objectCode[i].addr = search->loc;
			}

			// Immediate Addressing
			else if (objectCode[i].operand_first[0] == '#') {
				// format 3와 동일함. disp field에서 addr field로 바뀐 것만 차이가 있다.
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

		default:
			break;
		}

		unsigned int obj = 0;
		// format에 따라 obj를 결정한다.
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

		default:
			break;
		}
		objectCode[i].obj = obj;
	}
	return errorFlag;
}

// PASS1, PASS2를 하는 도중 error가 있으면 중간 자료 구조에 저장된 내용을 통해 보여준다.
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
			printf("%s %s %s %s\n", objectCode[i].label, objectCode[i].opcode, objectCode[i].operand_first, objectCode[i].operand_second);
		}
	}
}


// PASS1, PASS2를 모두 통과한다면 listing file과 object file을 모두 생성한다.
void writeFiles(char lstFile[], char objFile[]) {
	FILE* fp = fopen(lstFile, "w");
	FILE* ob = fopen(objFile, "w");
	if (fp == NULL || ob == NULL) {
		FILE_ERROR();
		return;
	}
	// Modifications 큐 생성
	HeadModifyRecord = (Modifications*)malloc(sizeof(Modifications));
	HeadModifyRecord->loc = 0;
	HeadModifyRecord->next = NULL;
	TailModifyRecord = HeadModifyRecord;

	/*-------------------------------------------------------------*/
	//##############################################################
	//
	// Listing File에 출력
	//
	/*-------------------------------------------------------------*/
	//###############################################################
	// 아래 for문은 단순히 objectCode 배열에 저장된 각 line의 특성에 따라 출력만 함.
	for (int i = 1; i <= assemble_end_line; i++) {
		int line = i * 5;
		Formats* forShow = &objectCode[i];
		
		
		if (forShow->isComment) {
			fprintf(fp, "%-8d                %s", line, forShow->comment);
			//printf("%-8d                %s", line, forShow->comment);
		}
		else if (forShow->isConstant) {
			fprintf(fp, "%-8d%04X    %-8s%-8s%-8s%-8s", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);
			//printf("%-8d%04X    %-8s%-8s%-8s%-8s", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);
			if (forShow->operand_first[0] == 'X') {
				char tmp[30];
				strcpy(tmp, forShow->operand_first + 2);
				tmp[strlen(tmp) - 1] = '\0';
				forShow->format = strlen(tmp) / 2;

				fprintf(fp, "%-s\n", tmp);
				//printf("%-s\n", tmp);

			}
			else if (forShow->operand_first[0] == 'C') {
				forShow->format = strlen(forShow->operand_first) - 3;
				fprintf(fp, "%X\n", forShow->obj);
				//printf("%X\n", forShow->obj);
			}
			else if (!strcmp(forShow->opcode, "WORD")) {
				forShow->format = 3;
				fprintf(fp, "%06X\n", forShow->obj);
				//printf("%06X\n", forShow->obj);

			}
		}
		else if (forShow->isVariable) {
			fprintf(fp, "%-8d%04X    %-8s%-8s%-8s%-8s\n", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);
			//printf("%-8d%04X    %-8s%-8s%-8s%-8s\n", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);

		}
		else if (forShow->isDirective) {
			fprintf(fp, "%-8d        %-8s%-8s%-8s%-8s\n", line, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);
			//printf("%-8d        %-8s%-8s%-8s%-8s\n", line, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);

		}
		else {
			fprintf(fp, "%-8d%04X    %-8s%-8s%-8s%-8s", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);
			//printf("%-8d%04X    %-8s%-8s%-8s%-8s", line, forShow->loc, forShow->label, forShow->opcode, forShow->operand_first, forShow->operand_second);

			switch (forShow->format) {
			case 1:
				fprintf(fp, "%02X\n", forShow->obj);
				//printf("%02X\n", forShow->obj);
				break;

			case 2:
				fprintf(fp, "%04X\n", forShow->obj);
				//printf("%04X\n", forShow->obj);				
				break;

			case 3:
				fprintf(fp, "%06X\n", forShow->obj);
				//printf("%06X\n", forShow->obj);
				break;

			// format 4에 대해서는 큐에 추가
			case 4:
				fprintf(fp, "%08X\n", forShow->obj);
				//printf("%08X\n", forShow->obj);	
				if (!(forShow->nixbpe ^ 0b110001)) {
					if (HeadModifyRecord == TailModifyRecord) {
						HeadModifyRecord->loc = forShow->loc;
						Modifications* temp = (Modifications*)malloc(sizeof(Modifications));
						temp->next = NULL;
						HeadModifyRecord->next = temp;
						TailModifyRecord = temp;
					}
					else {
						TailModifyRecord->loc = forShow->loc;
						Modifications* temp = (Modifications*)malloc(sizeof(Modifications));
						temp->next = NULL;
						TailModifyRecord->next = temp;
						TailModifyRecord = temp;
					}
				}
				break;
				
			default:
				fprintf(fp, "\n");
				//printf("\n");
				break;
			}
		}
	}

	/*-------------------------------------------------------------*/
	//##############################################################
	//
	// Object File에 출력
	//
	/*-------------------------------------------------------------*/
	//###############################################################


	// Header field 출력

	fprintf(ob, "H%-6s%06X%06X\n", objectCode[assemble_start_line].label, objectCode[assemble_start_line].loc, objectCode[assemble_end_line].loc);
	//printf("H%-6s%06X%06X\n", objectCode[assemble_start_line].label, objectCode[assemble_start_line].loc, objectCode[assemble_end_line].loc);


	// Text field 출력
	for (int i = assemble_start_line + 1; i <= assemble_end_line;) {
		if (objectCode[i].isComment || objectCode[i].isDirective || objectCode[i].isVariable) {
			i++; continue;
		}
		fprintf(ob, "T%06X", objectCode[i].loc);
		//printf("T%06X", objectCode[i].loc);

		
		// 최대 출력 비트 수가 30이므로 거리 계산을 한다.
		// s는 시작 인덱스, e는 끝 인덱스
		int length = 0;
		int s = i;
		while (i <= assemble_end_line && !objectCode[i].isVariable) {
			if (objectCode[i].isComment || objectCode[i].isDirective) {
				i++; continue;
			}
			length += objectCode[i].format;
			if (length >= 30) {
				length -= objectCode[i].format;
				break;
			}
			i++;
		}
		int e = i;
		fprintf(ob, "%02X", length);
		//printf("%02X", length);
		for (int j = s; j < e; j++) {
			if (objectCode[j].isConstant) {
				if (objectCode[j].operand_first[0] == 'X') {
					char tmp[30];
					strcpy(tmp, objectCode[j].operand_first + 2);
					tmp[strlen(tmp) - 1] = '\0';
					fprintf(ob, "%-s", tmp);
					//printf("%-s", tmp);

				}
				else if (objectCode[j].operand_first[0] == 'C') {
					fprintf(ob, "%X", objectCode[j].obj);
					//printf("%X", objectCode[j].obj);
				}
				else if(!strcmp(objectCode[j].opcode, "WORD")) {
					fprintf(ob, "%06X", objectCode[j].obj);
					//printf("%06X", objectCode[j].obj);
				}
				continue;
			}
			
			switch (objectCode[j].format) {
			case 1:
				fprintf(ob, "%02X", objectCode[j].obj);
				//printf("%02X", objectCode[j].obj);
				break;

			case 2:
				fprintf(ob, "%04X", objectCode[j].obj);
				//printf("%04X", objectCode[j].obj);
				break;

			case 3:
				fprintf(ob, "%06X", objectCode[j].obj);
				//printf("%06X", objectCode[j].obj);
				break;

			case 4:
				fprintf(ob, "%08X", objectCode[j].obj);
				//printf("%08X", objectCode[j].obj);
				break;
			}
		}
		fprintf(ob, "\n");
		//printf("\n");
	}
	
	// Modification field 출력
	for (Modifications* search = HeadModifyRecord; search != TailModifyRecord; search = search->next) {
		fprintf(ob, "M%06X%02d\n", search->loc + 1, 5);
		//printf("M%06X%02d\n", search->loc + 1, 5);
	}
	// End field 출력
	fprintf(ob, "E%06X\n", objectCode[assemble_start_line].loc);
	//printf("E%06X\n", objectCode[assemble_start_line].loc);

	// Modifications 큐 해제
	for (Modifications* search = HeadModifyRecord; search != TailModifyRecord; ) {
		if (search) {
			Modifications* temp = search->next;
			free(search);
			search = temp;
		}
		else break;
	}
	fclose(fp);
	fclose(ob);
}

void doAssemble(char parsedInstruction[][MAX_PARSED_NUM + 10]) {
	char temp[] = "lst";
	char temp2[] = "obj";
	// parsedInstruction[1]에는 *.asm에서 *에 해당하는 문자열이 있다.
	strcpy(lstFile, parsedInstruction[1]);
	strcpy(objFile, parsedInstruction[1]);
	int len = (int)strlen(lstFile);
	objFile[len - 3] = '\0';
	lstFile[len - 3] = '\0';
	strcat(lstFile, temp);
	strcat(objFile, temp2);

	symbolRoot = getNewSymbolNode();
	memset(objectCode, 0, sizeof(objectCode));

	// pass1, pass2를 모두 error 없이 통과해야 출력 가능하다.
	if (pass1(parsedInstruction[1]) == false) {
		if (pass2() == false) {
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
