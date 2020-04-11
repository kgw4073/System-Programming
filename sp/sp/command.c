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
int parameters[3] = { 0, };
// 마지막 dump한 메모리 주소를 저장함. 처음에는 -1로 초기화
int lastAddress = -1;

// 주어진 주소가 범위를 벗어났는지 확인한다.
inline bool isOverflowed(int address) {
	if (address < 0 || address > 0xfffff) return true;
	else return false;
}

// 주어진 문자를 10진수로 반환한다. 다만 hexa 범위를 벗어난 숫자가 들어오면 WRONG_HEXA를 반환하여 error 처리를 한다.
inline int toDecimal(char c) {
	if ('0' <= c && c <= '9') return c - '0';
	else if ('a' <= c && c <= 'f') return c - 'a' + 10;
	else if ('A' <= c && c <= 'F') return c - 'A' + 10;
	else return WRONG_HEXA;
}

// History 큐에 넣는 함수.
void insertHistory(char instruction[MAX_COMMAND_SIZE]) {
	historyNode* newHistoryNode = (historyNode*)malloc(sizeof(historyNode));
	newHistoryNode->command = NULL;
	// 아직 아무 것도 history에 없는 경우
	if (headOfHistory == tailOfHistory) {
		int len = (int)strlen(instruction);

		// headOfHistory에 추가한다.
		headOfHistory->command = (char*)malloc((size_t)(len + 1)*sizeof(char));
		strncpy(headOfHistory->command, instruction, (size_t)len);
		headOfHistory->command[len] = '\0';
		headOfHistory->next = newHistoryNode;
		tailOfHistory = newHistoryNode;
	}
	// 이미 history에 있는 경우
	else {
		int len = (int)strlen(instruction);

		// tailOfHistory에 추가한다.
		tailOfHistory->command = (char*)malloc((size_t)(len + 1) * sizeof(char));
		strncpy(tailOfHistory->command, instruction, (size_t)len);
		tailOfHistory->command[len] = '\0';
		tailOfHistory->next = newHistoryNode;
		tailOfHistory = newHistoryNode;
	}
}

// history queue에 할당된 메모리를 전부 해제한다.
void deleteHistory() {
	historyNode* temp = headOfHistory;
	if (!temp) return;
	if (temp->next) {
		temp = temp->next;
		if(headOfHistory->command) 
			free(headOfHistory->command);
		free(headOfHistory);
	}
	if (temp) {
		if(temp->command)
			free(temp->command);
		free(temp);
	}
}

// hi[story] 명령어를 입력했을 때 history 큐를 출력
void showHistory() {
	int count = 1;
	historyNode* temp = headOfHistory;
	while (temp != tailOfHistory) {
		printf("%-4d ", count++);
		printf("%s\n", temp->command);
		temp = temp->next;
	}
}

// 메모리를 dump함.
// 인자 :
// int parameters[]에는 2개의 유효한 인자가 있다.
// parameters[0]에는 start 주소가, parameters[1]에는 end의 주소가 있다.
void dumpMemory(int parameters[]) {

	// dump할 때는 16배수 단위로 dump해야 하기 때문.
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
			// 실제 parameters[0]과 parameters[1] 사이의 범위에 대해서 ascii code를 출력.
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

// h[elp]를 입력했을 때 명령어들을 보여줌.
void showHelp() {
	printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\n\
du[mp][start, end]\ne[dit] address, value\nf[ill] start, end, value\n\
reset\nopcode mnemonic\nopcodelist\nassemble filename\ntype filename\nsymbol\n");

}

// d[ir]을 입력했을 때 해당 폴더에 있는 파일들을 보여줌.
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
		// entry->d_name에 이름이 저장됨.
		strcpy(filename, entry->d_name);

		// 디렉토리일 때
		if (S_ISDIR(buf.st_mode)) {
			// "."이나 ".."이 아니면 뒤에 /를 붙여 디렉토리임을 표시.
			if (strcmp(entry->d_name, "..") && strcmp(entry->d_name, ".")) {
				strcat(filename, "/");
			}
		}
		// 실행파일이면 "*"을 붙임.
		else if((S_IXUSR & buf.st_mode)) {
			strcat(filename, "*");
		}
		// filename을 출력
		printf("%-20s", filename);

		// 한 라인에 4개씩 보여줌.
		if (++counter % 4 == 0) {
			printf("\n");
		}
	}
	if (counter % 4) printf("\n");
	closedir(dp);
	return;
}

// 메모리를 전부 0으로 초기화.
void resetMemory() {
	memset(vMemory, 0, sizeof(vMemory));
}

// 메모리 특정 주소를 수정.
void editMemory(int parameters[]) {
	// edit의 경우 parameters가 두 개가 넘어온다.
	// parameters[0]은 수정할 주소
	// parameters[1]은 수정할 값
	vMemory[parameters[0]] = (unsigned char)parameters[1];
}

// 메모리의 특정 부분을 특정 값으로 채움.
void fillMemory(int parameters[]) {
	// fill은 parameters가 세 개가 넘어온다.
	// parameters[0]은 시작 주소
	// parameters[1]은 끝 주소
	// parameters[2]는 채울 값
	for (int inch = parameters[0]; inch <= parameters[1]; inch++) {
		vMemory[inch] = (unsigned char)parameters[2];
	}
}

// opcode name라는 instruction이 들어왔을 때, 해당 opcode가 있는지 없는지를 반환.
// 인자 : OpCodeDecimal은 26진수로 봤을 때 나타내는 opcode의 10진수이다.
OpNode* findOpCodeNode(int OpCodeDecimal) {
	// hashEntry는 OpCodeDeciaml을 hash table의 사이즈로 나눈 나머지이다.
	int hashEntry = OpCodeDecimal % MAX_HASH_SIZE;
	OpNode* search = hashTable[hashEntry];
	// 연결 리스트를 따라가며 탐색
	while (1) {
		if (search == NULL) return NULL;
		if (search->decimal == OpCodeDecimal) {
			return search;
		}
		search = search->next;
	}
	return NULL;
}

// 인자 : parameters[]에는 하나의 인자만 넘어온다.
// parameters[0]은 해당 Mnemonic을 10진수로 변환한 값이 있음.
void showMnemonic(int parameters[]) {
	
	int OpCodeDecimal = parameters[0];
	OpNode* show;
	// 들어온 OpCOdeDecimal이 hash table에 있으면 출력
	// 없을 때는 Command parsing할 때 따로 예외처리 해두었음
	if ((show = findOpCodeNode(OpCodeDecimal)) != NULL) {
		printf("opcode is %02X\n", show->value);
	}
}

// opcode hash table을 전부 출력
void showOpcodelist() {
	for (int i = 0; i < MAX_HASH_SIZE; i++) {
		printf("%d : ", i);
		OpNode* inch = hashTable[i];
		// hashTailPointer, 즉, 각 hashEntry에 대해 끝에 도달했으면 다음 줄을 출력
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


void typeFile(char parsedInstruction[][MAX_PARSED_NUM + 10]) {
	FILE* fp = fopen(parsedInstruction[1], "r");
	if (fp == NULL) {
		FILE_ERROR();
		return;
	}
	char c;
	while (fscanf(fp, "%c", &c) != EOF) {
		printf("%c", c);
	}

	fclose(fp);
}




/* 
 * function : 정상적인 명령어가 들어와서 해당 명령어를 실행하는 함수
 * parameter :
 * 1. current_command : enum input_command로 구분돼 있음. (command.h 참조)
 */
void playCommand(char parsedInstruction[][MAX_PARSED_NUM + 10], enum input_command current_command) {
	switch (current_command) {
	case h:
	case help:
		showHelp();
		break;
	case d:
	case dir:
		showDir();
		break;

	case q:
	case quit:
		quit_flag = true;
		break;

	case hi:
	case history:
		showHistory();
		break;
	case du:
	case dump:
		dumpMemory(parameters);
		break;

	case e:
	case edit:
		editMemory(parameters);
		break;

	case f:
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

	case type:
		typeFile(parsedInstruction);
		break;

	case assemble:
		doAssemble(parsedInstruction);
		break;

	case symbol:
		
		break;
	}
}

/* 
 * function : 실행 가능한 명령어인지 판단하는 함수
 * parameter : 
 * 1. current_command : enum input_command로 구분되는 command
 * 2. parsedInstruction[][] : 이차원 배열로 instruction을 전달함. 각 행에는 공백으로 구분되는 문자열이 저장됨.
 *							예를 들어 dump 4, 37 이면 command[0]에는 "dump", command[1]에는 "4,", command[2]는 "37"이 있음 
 * 3. int* parsedReference : parsedNumber가 몇 개인지 저장하기 위해일종의 "call by reference"로 넘겨준 인자.
 */

RETURN_CODE isExecutable(enum input_command current_command, 
	char parsedInstruction[][MAX_PARSED_NUM + 10], int* parsedReference) {

	// start, last는 시작 주소와 끝 주소가 필요한 명령어인 경우 사용됨
	// valueDecimal은 input으로 들어온 값을 계산할 때 사용됨
	// hexaDecimal은 input으로 들어온 hexa를 계산할 때 사용됨
	int start = 0, last = 0, valueDecimal = 0, hexaDecimal = 0;
	// parsedNumber는 공백으로 구분되는 instruction의 개수를 저장하기 위해 사용됨.
	int parsedNumber = 0;

	// parsedInstruction의 첫 번째 값이 \0이면 break하여 parsedNumber가 몇 개인지 계산.
	for (int i = 0; ; i++) {
		if (!parsedInstruction[i][0]) break;
		parsedNumber++;
	}


	// command : dump
	if (current_command == dump) {
		hexaDecimal = 0;
		int lastIndex = (int)strlen(parsedInstruction[1]) - 1;

		// dump 만 들어왔을 때
		if (parsedNumber == 1) {
			// 이전에 출력한 마지막 주소 다음부터 출력해야 하므로 +1
			start = lastAddress + 1;
			// start가 범위를 넘어가면 0부터 시작함
			if (start > 0xfffff) start = 0;
			// 160개 출력
			lastAddress = (start + MAX_DUMP_BYTE * MAX_DUMP_LINE - 1);
			// lastAddress가 범위를 넘어가면 0xfffff 까지만
			if (lastAddress >= 0xfffff) lastAddress = 0xfffff;
			last = lastAddress;
			parameters[0] = start, parameters[1] = last, * parsedReference = parsedNumber;
		}

		// dump start 형태로 들어올 때
		else if (parsedNumber == 2) {
			// 가장 끝 문자부터 Decimal로 바꿈
			for (int i = lastIndex, j = 0; i >= 0; i--, j++) {
				// 읽어들였을 때 0~f에 해당하는 hexa가 아니면 잘못된 주소가 입력된 것이므로 에러를 발생.
				if ((hexaDecimal = toDecimal(parsedInstruction[1][i])) == WRONG_HEXA) {
					return ADDRESS_INPUT_ERROR;
				}
				// 아니면 계속해서 start를 계산.
				else {
					start += (int)pow(16, (double)j) * hexaDecimal;
				}
			}
			// hexa가 제대로 들어오긴 했을 때 범위를 넘는지 안 넘는지 검사.
			if (isOverflowed(start)) {
				return MEMORY_INDEX_ERROR;
			}

			else {
				last = start + MAX_DUMP_BYTE * MAX_DUMP_LINE - 1;
				if (last >= 0xfffff) last = 0xfffff;
				lastAddress = last;
			}
		}

		// dump start, end 형태로 들어옴.
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
			// start, 부분의 마지막 문자가 ','이 아니면 command 에러 
			else {
				return COMMAND_ERROR;
			}

			// end 부분을 파싱
			lastIndex = (int)strlen(parsedInstruction[2]) - 1;
			for (int i = lastIndex, j = 0; i >= 0; i--, j++) {
				// 0~f에 해당하는 hexa가 아니면 input error를 발생시킴.
				if ((hexaDecimal = toDecimal(parsedInstruction[2][i])) == WRONG_HEXA) {
					return ADDRESS_INPUT_ERROR;
				}
				else {
					last += (int)pow(16, (double)j) * hexaDecimal;
				}
			}
			// start와 last가 하나라도 범위를 벗어나는지, 그리고 start가 last보다 크면 index error를 발생시킴.
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

	// command : edit address, value
	else if (current_command == edit) {
		if (parsedNumber != 3) {
			return COMMAND_ERROR;
		}

		hexaDecimal = 0;
		int address = 0;
		int lastIndex = (int)strlen(parsedInstruction[1]) - 1;
		// address, 부분에서 마지막 문자가 ','여야 함.
		if (parsedInstruction[1][lastIndex] == ',') {
			for (int i = lastIndex - 1, j = 0; i >= 0; i--, j++) {
				if ((hexaDecimal = toDecimal(parsedInstruction[1][i])) == WRONG_HEXA) {
					return ADDRESS_INPUT_ERROR;
				}
				else {
					address += (int)pow(16, (double)j) * hexaDecimal;
				}
			}
		}

		else {
			return COMMAND_ERROR;
		}

		// value 계산
		lastIndex = (int)strlen(parsedInstruction[2]) - 1;
		for (int i = lastIndex, j = 0; i >= 0; i--, j++) {
			if ((hexaDecimal = toDecimal(parsedInstruction[2][i])) == WRONG_HEXA) {
				return ADDRESS_INPUT_ERROR;
			}
			else {
				valueDecimal += (int)pow(16, (double)j) * hexaDecimal;
			}
		}
		if (isOverflowed(address)) {
			return MEMORY_INDEX_ERROR;
		}
		if (valueDecimal < 0 || valueDecimal > 0xff) {
			return VALUE_ERROR;
		}
		parameters[0] = address, parameters[1] = valueDecimal;
	}


	// command : fill
	// fill은 start, end, value로 구성되어 있어야 함.
	else if (current_command == fill) {
		if (parsedNumber != 4) {
			return COMMAND_ERROR;
		}
		
		int hexaDecimal = 0;
		
		// start, 파싱
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

		// end, 파싱
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

		// value 파싱
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

	// command : opcode
	// opcode mnemonic 형태로 들어옴.
	else if (current_command == opcode) {
		if (parsedNumber != 2) return false;
		int len = (int)strlen(parsedInstruction[1]);
		int base = 1;
		int OpCodeDecimal = 0;
		for (int i = 0; i < len; i++) {
			int decimal = parsedInstruction[1][i] - 'A' + 1;
			// decimal이 영어 대문자로 들어와야만 OpCodeDecimal을 계산.
			if (decimal >= 0 && decimal <= 26) {
				OpCodeDecimal += (decimal * base);
				base *= 26;
			}
			// 영어 대문자가 아니면 OPCODE_ERROR 발생
			else {
				return OPCODE_ERROR;
			}
		}
		
		parameters[0] = OpCodeDecimal;
		return NORMAL;
	}

	else if (current_command == type || current_command == assemble) {
		if (parsedNumber != 2) return COMMAND_ERROR;

		FILE* f = fopen(parsedInstruction[1], "r");
		if (f == NULL) {
			return FILE_OPEN_ERROR;
		}
		fclose(f);
		return NORMAL;
	}


	// instruction이 command 단독인 경우.
	else {
		// instruction이 파싱된 후 문자열이 한 개가 아니면 COMMAND_ERROR 발생
		if (parsedNumber != 1) return COMMAND_ERROR;
	}
	*parsedReference = parsedNumber;
	return NORMAL;
}

/*
 * function : 입력된 instruction을 그대로 받아 여러 개의 공백을 하나의 공백으로 바꿔주고, 명령어가 입력되기 전 공백을 지움.
 * 
 * parameter : char instruction[] : fgets로 받은 문자열 그대로를 받아옴.
 */
void commandParse(char instruction[MAX_COMMAND_SIZE]) {
	char temp[MAX_COMMAND_SIZE];
	RETURN_CODE ret;
	
	// blank_buffer는 공백이 들어갔었는지 체크.
	// 일단 처음엔 공백이 없으므로 false로 초기화
	bool blank_buffer = false;
	// parsed_length는 공백을 제거하고 남은 진짜 instruction 문자열의 length
	int parsed_length = 0;
	for (int i = 0; i < MAX_COMMAND_SIZE; i++) {
		// 공백이 들어왔는데
		if (instruction[i] == ' ' || instruction[i] == '\t') {
			// 공백이 들어온 적이 없으면서 parsed instruction이 들어오고 있다면 공백 하나 넣어도 됨.
			if (!blank_buffer && parsed_length > 0) {
				// 
				blank_buffer = true;
				temp[parsed_length++] = instruction[i];
			}
		}
		else if (instruction[i] == '\0') {
			temp[parsed_length] = '\0';
			break;
		}
		// 공백이 아닌 문자가 들어오면 그냥 복사하고, blank_buffer를 false로 바꿈으로써 공백이 들어올 수 있게 함.
		else {
			temp[parsed_length++] = instruction[i];
			blank_buffer = false;
		}
	}
	memcpy(instruction, temp, sizeof(temp));


	// parsedInstruction은, 공백으로 구분되는 insturction[]을 각 파트에 따라 다른 행에 복사함.
	// 예를 들어 char instruction[] = "dump 4, 37"은
	// parsedInstruction[0] = "dump"
	// parsedInstruction[1] = "4,"
	// parsedInstruction[2] = "37"
	// 로 구분함.
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
	// pInstruction은 parsedInstruction[0]이 나타내는 문자열의 주소.

	int number;
	// Trie에 검색하였는데 없으면 에러.
	if ((number = searchTrie(root, parsedInstruction[0])) == 0) {
		ret = COMMAND_ERROR;
		goto ERROR_HANDLING;
	}
	
	current_command = number;
	if (current_command == quit) {
		quit_flag = true;
	}

	int parsedNumber = 0;
	ret = isExecutable(current_command, parsedInstruction, &parsedNumber);


// error 핸들링하는 label
ERROR_HANDLING:
	switch (ret) {
	case NORMAL:
		insertHistory(instruction);
		playCommand(parsedInstruction, current_command);
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

	case FILE_OPEN_ERROR:
		FILE_ERROR();
		break;
	}

}
