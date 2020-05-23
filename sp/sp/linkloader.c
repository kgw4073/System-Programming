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

// LoadMap은 Control Section에 대한 정보만 저장.
LoadMap* LoadmapRoot = NULL;

// ESTabRoot는 Control Section과 Symbol에 대한 정보 모두 저장.
ESTab* ESTabRoot = NULL;
int ProgramLengthForLink = 0;

// 재귀적으로 Symbol Queue를 할당 해제
void deleteSymbolQ(ExtSymbol* root) {
	if (root) {
		deleteSymbolQ(root->next);
		free(root);
	}
}

// 재귀적으로 Loadmap Binary Search Tree를 해제
void deleteLoadmap(LoadMap* root) {
	if (root) {
		deleteSymbolQ(root->front);
		deleteLoadmap(root->left);
		deleteLoadmap(root->right);
		free(root);
	}
}
// 재귀적으로 Control Section과 Symbol에 대한 정보를 담고 있는 BST를 해제
void deleteESTabRoot(ESTab* root) {
	if (root) {
		deleteESTabRoot(root->left);
		deleteESTabRoot(root->right);
		free(root);
	}
}

// ESTab Tree에 ControlSection이름을 가진 노드가 있는지 체크.
// Binary Search로 구현
ESTab* findESTab(ESTab* root, char ControlSection[]) {
	if (root) {
		int res = strcmp(root->ControlSection, ControlSection);
		if (res > 0) {
			return findESTab(root->left, ControlSection);
		}
		else if (res < 0) {
			return findESTab(root->right, ControlSection);
		}
		else return root;
	}
	else return NULL;
}

// Extern Symbol Node를 새로이 할당
ExtSymbol* getNewExtSymbolNode() {
	ExtSymbol* temp = (ExtSymbol*)malloc(sizeof(ExtSymbol));
	memset(temp->Symbol, 0, sizeof(temp->Symbol));
	temp->addr = 0;
	return temp;
}

// ESTab Tree에 들어갈 Node를 새로이 할당
ESTab* getNewESTabNode() {
	ESTab* temp = (ESTab*)malloc(sizeof(ESTab));
	temp->ControlSection[0] = '\0';
	temp->CSAddr = 0;
	temp->length = -1;
	temp->left = temp->right = NULL;
	return temp;
}

// LoadMap Tree에 들어갈 Node를 새로이 할당
LoadMap* getNewLoadMapNode() {
	LoadMap* temp = (LoadMap*)malloc(sizeof(LoadMap));
	memset(temp->ControlSection, 0, sizeof(temp->ControlSection));
	temp->CSAddr = 0;
	// length를 불가능한 값으로 설정해서 빈 노드를 판별함.
	temp->length = -1;
	temp->left = temp->right = NULL;
	temp->front = temp->rear = NULL;
	return temp;
}

// Binary Search Tree로 Loadmap Tree에 정보를 삽입함.
LoadMap* insertLoadMap(LoadMap* root, char ControlSection[], int CSAddr, int length) {
	// Loadmap 을 새로이 할당할 때 length를 -1로 설정하였는데 이는 빈 노드를 가리킨다.
	if (root->length == -1) {
		strcpy(root->ControlSection, ControlSection);
		root->CSAddr = CSAddr;
		root->length = length;
		return root;
	}
	int res = strcmp(root->ControlSection, ControlSection);
	// 왼쪽 서브 트리에 삽입
	if (res > 0) {
		if (!root->left) {
			LoadMap* temp = getNewLoadMapNode();
			root->left = temp;
		}
		return insertLoadMap(root->left, ControlSection, CSAddr, length);
	}
	// 오른쪽 서브 트리에 삽입
	else if (res < 0) {
		if (!root->right) {
			LoadMap* temp = getNewLoadMapNode();
			root->right = temp;
		}
		return insertLoadMap(root->right, ControlSection, CSAddr, length);
	}
	// 겹치는 노드가 있는 것이므로 NULL을 반환하여 에러 발생.
	else return NULL;
}

// ESTab도 Loadmap과 마찬가지로 BST에 삽입.
void insertESTab(ESTab* root, char ControlSection[], int CSAddr, int length) {
	// 빈 노드이면 해당 노드가 삽입할 위치가 됨.
	if (root->length == -1) {
		strcpy(root->ControlSection, ControlSection);
		root->CSAddr = CSAddr;
		root->length = length;
		return;
	}
	int res = strcmp(root->ControlSection, ControlSection);
	// 왼쪽 서브 트리에 삽입
	if (res > 0) {
		if (!root->left) {
			ESTab* temp = getNewESTabNode();
			root->left = temp;
		}
		insertESTab(root->left, ControlSection, CSAddr, length);
	}
	// 오른쪽 서브 트리에 삽입
	else if (res < 0) {
		if (!root->right) {
			ESTab* temp = getNewESTabNode();
			root->right = temp;
		}
		insertESTab(root->right, ControlSection, CSAddr, length);
	}
}

// LoadMap Tree의 한 노드에는 Control Section에 대한 정보가 저장되어 있음.
// 해당 노드에는 Extern Symbol을 저장할 수 있는 큐가 있음.
void insertLoadMapQ(LoadMap* root, char name[], int nodeAddr) {
	if (!root->rear) {
		root->front = getNewExtSymbolNode();
		root->front->next = getNewExtSymbolNode();
		strcpy(root->front->Symbol, name);
		root->front->addr = nodeAddr;
		root->rear = root->front->next;
	}
	else {
		strcpy(root->rear->Symbol, name);
		root->rear->addr = nodeAddr;
		root->rear->next = getNewExtSymbolNode();
		root->rear = root->rear->next;
	}
}

// Linking Pass1
// 인자 : filenames들
// 반환 값 : boolean (에러가 있으면 true를, 없으면 false를 반환)
bool LinkingPass1(char filenames[][MAX_PARSED_NUM + 10]) {

	// CSADDR_BASE는 메모리에 로딩할 위치
	int CSADDR_BASE = Progaddr;

	// ESTab, Loadmap의 트리 루트를 생성.
	ESTabRoot = getNewESTabNode();
	LoadmapRoot = getNewLoadMapNode();

	char line[1000];
	char* pLine;
	int CurrentSectionLength;
	int ProgramTotalLength = 0;
	LoadMap* CurrentLoadMap;
	
	
	for (int i = 1; ; i++) {
		// command로 loader [1] [2] [3] ... 이 들어올 수 있음.
		// NULL String에 도달하면 linking을 종료함.
		if (filenames[i][0] == '\0') {
			break;
		}
		FILE* fp = fopen(filenames[i], "r");
		while (1) {
			// 한 줄을 읽어 들인다.
			line[0] = '\0';
			fgets(line, 1000, fp);
			// E이면 종료
			if (line[0] == 'E') break;
			// Comment이면 다음 줄 읽음.
			if (line[0] == '.') continue;

			// Header Record
			if (line[0] == 'H') {
				pLine = line + 1;
				char ControlSection[100], Start[100], Length[100];
				// Control Section name, Start Address, Section Length를 입력 받음.
				sscanf(pLine, "%6s%6s%6s", ControlSection, Start, Length);

				// Control Section의 Address는 Start Address + CSADDR_BASE
				int CSAddr = strtol(Start, NULL, 16) + CSADDR_BASE;
				CurrentSectionLength = strtol(Length, NULL, 16);
				ProgramTotalLength += CurrentSectionLength;

				// ESTab Tree에 Control Section이름이 있는지 확인. 없으면 진행, 있으면 에러.
				ESTab* search = findESTab(ESTabRoot, ControlSection);

				// 없어야 삽입할 수 있음.
				if (search == NULL) {
					insertESTab(ESTabRoot, ControlSection, CSAddr, CurrentSectionLength);

					// Loadmap Tree에도 삽입할 수 있는지 판별.
					CurrentLoadMap = insertLoadMap(LoadmapRoot, ControlSection, CSAddr, CurrentSectionLength);
					// 단순 예외처리를 위함. 만약 기존에 삽입이 되었다면 NULL이 반환되며 에러를 발생시킴.
					if (CurrentLoadMap == NULL) {
						return true;
					}
				}
				// 에러 발생.
				else return true;
			}

			// Data Record
			else if (line[0] == 'D') {
				pLine = line + 1;
				while (1) {
					char name[100], addr[100];
					name[0] = '\0', addr[0] = '\0';
					// Symbol Name과 Control Section 안에서의 offset을 입력 받음.
					sscanf(pLine, "%6s%6s", name, addr);
					if (name[0] == '\0' || addr[0] == '\0') break;
					// nodeAddr은 Control Section Base + offset
					int nodeAddr = strtol(addr, NULL, 16) + CSADDR_BASE;

					// ESTab Tree에 Symbol name이 없어야 삽입 가능함.
					ESTab* search = findESTab(ESTabRoot, name);
					if (search == NULL) {
						insertESTab(ESTabRoot, name, nodeAddr, 0);
						insertLoadMapQ(CurrentLoadMap, name, nodeAddr);
						pLine += 12;
					}
					else return true;

				}
			}
		}
		// 한 Control Section이 끝나면 CS_BASE에 해당 CS의 Length만큼 더해주어야 함
		CSADDR_BASE += CurrentSectionLength;
		fclose(fp);
	}

	// Pass1이 정상적으로 끝나면 LoadMap을 출력함.
	PrintPass1Info_LoadMap(ProgramTotalLength);
	return false;
}

// LoadMap을 출력함.
void PrintPass1Info_LoadMap(int ProgramTotalLength) {
	printf("control symbol address length\nsection name\n");
	printf("--------------------------------\n");
	PrintLoadMap(LoadmapRoot);
	printf("--------------------------------\n");
	printf("           total length %04X\n", ProgramTotalLength);
	
	ProgramLengthForLink = ProgramTotalLength;
}

// 현재 Control Section에 있는 Symbol을 모두 출력.
void PrintCurrentLoadMapQ(LoadMap* root) {
	ExtSymbol* search = root->front;
	while (search != root->rear) {
		printf("        %6s   %04X\n", search->Symbol, search->addr);
		search = search->next;
	}
}
// LoadMap을 재귀적으로 출력함.
void PrintLoadMap(LoadMap* root) {
	if (root) {
		PrintLoadMap(root->left);
		printf("%s            %04X   %04X\n", root->ControlSection, root->CSAddr, root->length);
		PrintCurrentLoadMapQ(root);
		PrintLoadMap(root->right);
	}
}

// 실제 메모리 로딩, 재배치, 링킹을 담당하는 Pass2
// 인자 : 링킹할 목적파일명
// 반환 : boolean (에러가 있으면 true, 없으면 false)
bool LinkingPass2(char filenames[][MAX_PARSED_NUM + 10]) {

	// 지역 변수는 Pass1과 거의 동일
	// 그러나 END_EXECADDR과 START_EXECADDR을 정해야함.
	int CSADDR_BASE = Progaddr;
	END_EXECADDR = START_EXECADDR = Progaddr;
	
	char* pLine;
	char line[1000], name[100], ControlSection[100], Temp[100], Start[100], Length[100];
	int CurrentSectionLength, index, value, location, length;

	for (int i = 1; ; i++) {
		if (filenames[i][0] == '\0') {
			break;
		}
		FILE* fp = fopen(filenames[i], "r");
		while (1) {
			memset(line, 0, sizeof(line));
			fgets(line, 1000, fp);

			// END Record
			if (line[0] == 'E') {
				pLine = line + 1;
				Temp[0] = '\0';
				sscanf(pLine, "%6s", Temp);
				// E뒤에 뭐가 있으면 그곳이 해당 프로그램의 실행 시작 주소가 됨.
				if (Temp[0] != '\0') {
					START_EXECADDR = CSADDR_BASE + strtol(Temp, NULL, 16);
				}
				// END_EXECADDR은 CS의 길이를 더함.
				END_EXECADDR += CurrentSectionLength;
				break;
			}
			// Comment면 넘어감
			if (line[0] == '.') continue;

			// Head Record
			if (line[0] == 'H') {
				// Relocation Table의 1번은 현재 Control Section의 시작 주소임.
				Relocations[1] = CSADDR_BASE;
				pLine = line + 1;
				// Control Section, Start address, Length를 읽어 들임.
				sscanf(pLine, "%6s%6s%6s", ControlSection, Start, Length);
				CurrentSectionLength = strtol(Length, NULL, 16);
			}
			// Data Record이면 이미 Pass1에서 읽었기 때문에 패스
			else if (line[0] == 'D') {
				continue;
			}
			// 해당 Control Sectino에서 사용할 External Symbol를 담고 있음
			else if (line[0] == 'R') {
				pLine = line + 1;
				while (1) {
					Temp[0] = '\0';
					// 인덱스를 담은 Temp와 해당 Symbol name을 읽어들임.
					sscanf(pLine, "%2s%6s", Temp, name);
					if (Temp[0] == '\0') break;
					index = strtol(Temp, NULL, 10);
					pLine += 8;

					// ESTab Tree에 없으면 에러임.
					ESTab* search = findESTab(ESTabRoot, name);
					if (search == NULL) {
						return true;
					}
					
					// 현재 Control Section에서 사용할 Relocation table은 Relocations에 저장.
					Relocations[index] = search->RELOC_ADDR;
				}
			}

			// Text Record;
			else if (line[0] == 'T') {
				pLine = line + 1;
				// 시작 주소와 길이를 읽어들인다.
				sscanf(pLine, "%6s%2s", Start, Length);
				location = strtol(Start, NULL, 16) + CSADDR_BASE;
				length = strtol(Length, NULL, 16);
				pLine += 8;
				// 길이만큼 1 byte 씩 읽어서 메모리에 로딩한다.
				for (int k = 0; k < length; k++) {
					sscanf(pLine, "%2s", Temp);
					value = strtol(Temp, NULL, 16);
					vMemory[location + k] = (unsigned char)value;
					pLine += 2;
				}
			}

			// Modification Record
			else if (line[0] == 'M') {
				pLine = line + 1;

				sscanf(pLine, "%6s%2s", Start, Length);
				// Modification할 주소와, half byte의 길이를 읽어들인다.
				location = strtol(Start, NULL, 16) + CSADDR_BASE;
				length = strtol(Length, NULL, 16);
				pLine += 8;
				// Big endian으로 target을 계산.
				int target = (int)vMemory[location + 2] + 256 * (int)vMemory[location + 1] + 256 * 256 * (int)vMemory[location];
				if (pLine[0] == '+') {
					pLine++;
					sscanf(pLine, "%2d", &index);
					target += Relocations[index];
				}
				else if (pLine[0] == '-') {
					pLine++;
					sscanf(pLine, "%2d", &index);
					target -= Relocations[index];
				}
				unsigned char* pMemory = vMemory + location;

				// 5 half bytes이면 
				if (length == 5) {
					
					// 가장 오른쪽에서부터 5 half bytes만 고쳐야 하므로 다음과 같이 비트 연산
					pMemory[0] &= 0b11110000;
					pMemory[1] = pMemory[2] = 0;
					
					// target값을 오른쪽 5 half bytes에 or 연산을 통해 로딩함.
					for (int j = 0; j < 20; j++) {
						if (target & (1 << (19 - j))) {
							pMemory[(j + 4) / 8] |= (1 << ((19 - j) % 8));
						}
					}
				}
				
				// 6 half bytes를 바꾼다.
				else if (length == 6) {
					// 3 bytes모두 0으로 초기화 하고 target을 그대로 or 연산으로 옮겨야 함.
					pMemory[0] = pMemory[1] = pMemory[2] = 0;

					// pMemory를 시작으로 6 half bytes를 target에 로딩함.
					for (int j = 0; j < 24; j++) {
						if (target & (1 << (23 - j))) {
							pMemory[j / 8] |= (1 << ((23 - j) % 8));
						}
					}
				}
			}
		}
		// 한 Control Section 다 읽었으면 Control Section의 길이를 BASE에 더함.
		CSADDR_BASE += CurrentSectionLength;
		fclose(fp);
	}
	return false;
}

// 현재 실행되는 주소가 BreakPoint에 있는지 Linear Search로 판별한다.
bool isInBreakPointList(int current_addr) {
	for (int i = 0; i < sizeOfBP; i++) {
		if (BPlist[i] == current_addr) {
			return true;
		}
	}
	return false;
}

// op의 format을 반환한다.
int WhatIsFormat(int op) {
	switch (op) {
	case FIX:
	case FLOAT:
	case HIO:
	case SIO:
	case TIO:
		return 1;
	case ADDR:
	case CLEAR:
	case COMPR:
	case DIVR:
	case MULR:
	case RMO:
	case SHIFTL:
	case SUBR:
	case SVC:
	case TIXR:
		return 2;
	default:
		return 3;
	}
}

// 인자 : format, ProgramCounter(현재 실행 주소), opcode, nixbpe, displacement(12bits) or address(20bits),
//			sign_flag(+인지 -인지), r1, r2
// 기능 : 주어진 인자에 따라 명령어 별로 세부사항 실행.
void Execute(int format, int* ProgramCounter, int op, int nixbpe, int disp, int sign_flag, int r1, int r2) {

	// Jump를 하는지 안하는지 나타내는 flag
	bool JumpFlag = false;
	int address, target;

	// format 2
	if (format == 2) {
		switch (op) {
			// CLEAR라면 r1 register을 0으로 설정.
		case CLEAR:
			Register[r1] = 0;
			break;

			// copy.obj에서는 COMPR으로 r1과 r2를 실제로 비교하는 것이 아니기 때문에 SW reg를 0으로 바로 세팅한다.
		case COMPR:
			/*if (Register[r1] - Register[r2] > 0) {
				Register[SW] = 1;
			}
			else if (Register[r1] - Register[r2] == 0) {
				Register[SW] = 0;
			}
			else Register[SW] = -1;*/
			Register[SW] = 0;
			break;
			
			
		case TIXR:
			// X reg를 하나 증가시키고 X와 r1을 비교하여 SW reg를 세팅함.
			Register[X]++;
			if (Register[X] - Register[r1] > 0) {
				Register[SW] = 1;
			}
			else if (Register[X] - Register[r1] == 0) {
				Register[SW] = 0;
			}
			else Register[SW] = -1;
			break;
		}
	}

	// format 3
	else if (format == 3) {
		switch (op) {

			// L reg에 있는 값을 표현된 메모리 주소에 저장.
		case STL:
			// pc relative
			if (nixbpe & (1 << 1)) {
				unsigned char* pMem = &vMemory[*ProgramCounter + format + disp * sign_flag];
				*pMem = *(pMem + 1) = *(pMem + 2) = 0;

				// L register에 있는 값을 or 연산을 통해 로드함.
				for (int i = 0; i < 24; i++) {
					int t;
					if ((t = Register[L] & (1 << (23 - i)))) {
						*(pMem + (i / 8)) |= (t >> ((23 - i) / 8 * 8));
					}
				}
			}
			// base relative
			else if (nixbpe & (1 << 2)) {
				unsigned char* pMem = &vMemory[Register[B] + disp * sign_flag];
				*pMem = *(pMem + 1) = *(pMem + 2) = 0;
				for (int i = 0; i < 24; i++) {
					int t;
					if ((t = Register[L] & (1 << (23 - i)))) {
						*(pMem + (i / 8)) |= (t >> ((23 - i) / 8 * 8));
					}
				}
			}
			break;

			// 메모리 주소에 있는 값을 Base register에 올림.
		case LDB:
			// pc relative
			if (nixbpe & (1 << 1)) {
				// immediate이므로 나타내어진 address를 그대로 올림.
				if (nixbpe & (1 << 4)) {
					int address = *ProgramCounter + format + disp * sign_flag;
					Register[B] = address;
				}
			}

			break;

			// 메모리 주소에 있는 값을 A register로 올림.
		case LDA:
			// pc relative
			if (nixbpe & (1 << 1)) {
				int address = *ProgramCounter + format + disp * sign_flag;

				// immediate addressing
				if ((nixbpe & (1 << 4)) && !(nixbpe & (1 << 5))) {
					Register[A] = address;
				}

				// simple addressing
				else if ((nixbpe & (1 << 4)) && (nixbpe & (1 << 5))) {
					// Big endian
					Register[A] = 256 * 256 * vMemory[address] + 256 * vMemory[address + 1] + vMemory[address + 2];
				}
			}
			// base relative
			else if (nixbpe & (1 << 2)) {
				int address = Register[B] + disp * sign_flag;
				// immediate
				if ((nixbpe & (1 << 4)) && !(nixbpe & (1 << 5))) {
					Register[A] = address;
				}

				// simple addressing
				else if ((nixbpe & (1 << 4)) && (nixbpe & (1 << 5))) {
					Register[A] = 256 * 256 * vMemory[address] + 256 * vMemory[address + 1] + vMemory[address + 2];
				}
			}
			else {
				// immediate addressing
				if ((nixbpe & (1 << 4)) && !(nixbpe & (1 << 5))) {
					Register[A] = disp;
				}
			}
			break;

			// COMP는 copy.obj에서 immediate 로만 사용됨.
		case COMP:
			// immediate addressing
			if ((nixbpe & (1 << 4)) && !(nixbpe & (1 << 5))) {
				if (Register[A] > disp) {
					Register[SW] = 1;
				}
				else if (Register[A] < disp) {
					Register[SW] = -1;
				}
				else {
					Register[SW] = 0;
				}
			}
			break;

			// SW reg가 = 이면 해당 위치로 Jump한다.
		case JEQ:
			if (Register[SW] == 0) {
				JumpFlag = true;
				// pc relative
				if (nixbpe & (1 << 1)) {
					// simple addressing
					if ((nixbpe & (1 << 4)) && (nixbpe & (1 << 5))) {
						*ProgramCounter = *ProgramCounter + format + sign_flag * disp;
					}

				}
				// base relative
				else if (nixbpe & (1 << 2)) {
					*ProgramCounter = Register[B] + sign_flag * disp;
				}
			}
			break;

			// 표현되는 주소로 Jump한다.
		case J:

			JumpFlag = true;
			// pc relative
			if (nixbpe & (1 << 1)) {
				// simple addressing
				if ((nixbpe & (1 << 4)) && (nixbpe & (1 << 5))) {
					*ProgramCounter = *ProgramCounter + format + sign_flag * disp;
				}
				// indirect
				else if (nixbpe & (1 << 5)) {
					address = *ProgramCounter + format + sign_flag * disp;
					target = 256 * 256 * vMemory[address] + 256 * vMemory[address + 1] + vMemory[address + 2];
					*ProgramCounter = target;
				}

			}
			// base relative
			else if (nixbpe & (1 << 2)) {
				address = Register[B] + sign_flag * disp;
			}
			break;

			// A reg에 있는 값을 표현되는 메모리 주소에 저장.
		case STA:
			// pc relative
			if (nixbpe & (1 << 1)) {
				// 해당 메모리 값을 0으로 초기화 한 다음 or연산을 통해 저장.
				unsigned char* pMem = &vMemory[*ProgramCounter + format + disp * sign_flag];
				*pMem = *(pMem + 1) = *(pMem + 2) = 0;
				for (int i = 0; i < 24; i++) {
					int t;
					if ((t = Register[A] & (1 << (23 - i)))) {
						*(pMem + i / 8) |= t >> ((23 - i) / 8 * 8);
					}
				}
			}
			// base relative
			else if (nixbpe & (1 << 2)) {
				// 해당 메모리 값을 0으로 초기화 한 다음 or연산을 통해 저장.
				unsigned char* pMem = &vMemory[Register[B] + disp * sign_flag];
				*pMem = *(pMem + 1) = *(pMem + 2) = 0;
				for (int i = 0; i < 24; i++) {
					int t;
					if ((t = Register[A] & (1 << (23 - i)))) {
						*(pMem + i / 8) |= t >> ((23 - i) / 8 * 8);
					}
				}
			}
			break;

			// 표현된 메모리 주소에 있는 값 혹은 주소 자체를 T reg에 올림.
		case LDT:
			// pc relative
			if (nixbpe & (1 << 1)) {
				int address = *ProgramCounter + format + disp * sign_flag;

				// immediate
				if ((nixbpe & (1 << 4)) && !(nixbpe & (1 << 5))) {
					Register[T] = address;
				}
				// simple addressing
				else if ((nixbpe & (1 << 4)) && (nixbpe & (1 << 5))) {
					Register[T] = 256 * 256 * vMemory[address] + 256 * vMemory[address + 1] + vMemory[address + 2];
				}
			}
			// base relative
			else if (nixbpe & (1 << 2)) {
				int address = Register[B] + disp * sign_flag;
				if ((nixbpe & (1 << 4)) && !(nixbpe & (1 << 5))) {
					Register[T] = address;
				}

				// simple addressing
				else if ((nixbpe & (1 << 4)) && (nixbpe & (1 << 5))) {
					Register[T] = 256 * 256 * vMemory[address] + 256 * vMemory[address + 1] + vMemory[address + 2];
				}
			}
			
			break;

			// 본 프로그램에서는 < 로 SW를 설정.
		case TD:
			Register[SW] = -1;
			break;

			// 본 프로그램에서는 아무것도 하지 않음.
		case RD:
			break;

			// A reg의 가장 오른쪽 byte를 표현된 메모리에 올림.
			// 사실상 X reg를 index로 쓰기 때문에 Base relative임.
		case STCH:
			// index 사용
			if (nixbpe & (1 << 3)) {
				// base relative
				if (nixbpe & (1 << 2)) {
					unsigned char temp = (unsigned char)Register[A];
					address = Register[B] + Register[X] + disp * sign_flag;
					vMemory[address] = temp;
				}
			}
			break;

			// SW가 -1이면 Jump
		case JLT:
			if (Register[SW] == -1) {
				JumpFlag = true;
				// pc relative
				if (nixbpe & (1 << 1)) {
					address = *ProgramCounter + format + disp * sign_flag;
				}
				// base relative
				else if (nixbpe & (1 << 2)) {
					address = Register[B] + disp * sign_flag;
				}
				*ProgramCounter = address;
			}
			break;

			// X reg에 있는 값을 메모리에 저장.
		case STX:
			// pc relative
			if (nixbpe & (1 << 1)) {
				// 여타 Store과 관련한 instruction처럼 메모리 3bytes를 0으로 초기화하고 or연산으로 Register[X]를 저장.
				unsigned char* pMem = &vMemory[*ProgramCounter + format + disp * sign_flag];
				*pMem = *(pMem + 1) = *(pMem + 2) = 0;

				for (int i = 0; i < 24; i++) {
					int t;
					if ((t = (Register[X] & (1 << (23 - i))))) {
						*(pMem + i / 8) |= (t >> ((23 - i) / 8 * 8));
					}
				}

			}
			// base relative
			else if (nixbpe & (1 << 2)) {
				// Store과 관련한 instruction처럼 메모리 3bytes를 0으로 초기화하고 or연산으로 Register[X]를 저장.
				unsigned char* pMem = &vMemory[Register[X] + disp * sign_flag];
				*pMem = *(pMem + 1) = *(pMem + 2) = 0;

				for (int i = 0; i < 24; i++) {
					int t;
					if ((t = (Register[X] & (1 << (23 - i))))) {
						*(pMem + i / 8) |= (t >> ((23 - i) / 8 * 8));
					}
				}
			}
			break;

			// L reg에 있는 값을 Jump할 주소로 지정.
		case RSUB:
			*ProgramCounter = Register[L];
			JumpFlag = true;
			break;

			// LDCH는 메모리에 있는 값을 A의 오른쪽 bytes에 올림.
			// STCH와 비슷하게 X를 index register로 쓰기 때문에 base relative만 처리
		case LDCH:
			// 오른쪽 1 bytes를 0으로 초기화.
			Register[A] &= (0xFFFF00);
			// index
			if (nixbpe & (1 << 3)) {
				// base relative
				if (nixbpe & (1 << 2)) {
					address = Register[B] + Register[X] + disp * sign_flag;
					for (int i = 0; i < 8; i++) {
						if (vMemory[address] & (1 << i)) {
							Register[A] |= (1 << i);
						}
					}
				}
			}
			break;

			// 본 프로그램에서 WD는 아무것도 하지 않음
		case WD:
			break;

		default:
			break;
		}
	}

	// format 4
	else if (format == 4) {
		switch (op) {

			// PC를 L reg에 저장하고 메모리에 표현된 주소로 Jump함.
		case JSUB:
			JumpFlag = true;
			Register[L] = *ProgramCounter + format;

			// format 4에서는 disp를 주소 그대로 사용함.
			*ProgramCounter = disp;
			break;

			// disp에 있는 값을 그대로 T reg에 올림.
		case LDT:
			// immediate addressing
			if (nixbpe & (1 << 4)) {
				Register[T] = disp;
			}
			break;
		}
	}

	// Jump를 안하면 그냥 PC를 format만큼 더한다.
	if (JumpFlag == false) {
		*ProgramCounter += format;
	}
	// 새로운 PC를 저장.
	Register[PC] = *ProgramCounter;
}

// Register 정보를 출력
void PrintCurrentRegisters() {
	printf("A  : %06X   X  : %06X\n", Register[A], Register[X]);
	printf("L  : %06X  PC  : %06X\n", Register[L], Register[PC]);
	printf("B  : %06X   S  : %06X\n", Register[B], Register[S]);
	printf("T  : %06X\n", Register[T]);
}

// run을 입력하면 실행되는 함수.
void Run() {
	// Pass1, Pass2를 거쳐 계산된 START_EXECADDR을 current_addr로 시작함.
	int current_addr = START_EXECADDR;

	// 프로그램이 시작할 때 PC는 current_addr로, L은 프로그램의 길이로 설정.
	Register[PC] = current_addr;
	Register[L] = ProgramLengthForLink;

	// current_addr이 END_EXECADDR보다 커지면 루프 종료.
	while (1) {
		if (current_addr >= END_EXECADDR) {
			// End시에 Reg정보 출력.
			PrintCurrentRegisters();
			printf("            End Program\n");
			break;
		}
		// 루프를 돌 때마다 Breakpoint list에 있는지 확인.
		// breakpoint이면 'run'을 입력해야만 진행 가능.
		if (isInBreakPointList(current_addr)) {
			PrintCurrentRegisters();
			printf("            Stop at checkpoint[%X]\n", current_addr);

			char command[100];
			fgets(command, 100, stdin);
			command[strlen(command) - 1] = '\0';
			if (strcmp(command, "run")) {
				printf("계속 실행하려면 run을 입력하세요.\n");
				continue;
			}
		}

		// op, nixbpe, r1, r2, disp를 파싱하기 위한 지역 변수.
		int cur1 = vMemory[current_addr], cur2, cur3, cur4;
		int object_code = 0;
		int r1 = 0, r2 = 0, nixbpe = 0, disp = 0, sign_flag = 1;

		// 우선 op는 vMemory[currend_addr]로 설정.
		int op = cur1;
		int format = WhatIsFormat(op);

		// format 1은 본 프로그램에 존재하지 않음.
		if (format == 1) {
			continue;
		}

		// format 2
		else if (format == 2) {
			cur2 = vMemory[current_addr + 1];
			object_code = 256 * cur1 + cur2;
			r1 |= (cur2 & 0xF0);
			r1 >>= 4;
			r2 |= (cur2 & 0xF);
		}

		// format 3
		else if (format == 3) {
			// op는 6 bit이므로 >>=2와 <<=2를 해주어서 하위 2 bytes를 0으로 만듦.
			op >>= 2;
			op *= 4;
			cur2 = vMemory[current_addr + 1];
			cur3 = vMemory[current_addr + 2];
			object_code = 256 * 256 * cur1 + 256 * cur2 + cur3;

			// format 4
			if (cur2 & (1 << 4)) {
				format = 4;
				cur4 = vMemory[current_addr + 3];
				object_code = 256 * object_code + cur4;
				nixbpe |= ((object_code & 0x3F00000) >> 20);
				disp |= (object_code & 0xFFFFF);
			}

			// format 3
			else {
				nixbpe |= ((object_code & 0x3F000) >> 12);
				disp |= (object_code & 0xFFF);

				// disp 음수면 양수로 바꿔주고 sign_flag를 -1로 설정하자.
				if (disp & (1 << 11)) {
					disp = ~disp;
					disp &= 0x00000FFF;
					disp++;
					sign_flag = -1;
				}
			}
		}

		// 주어진 op, format, current_addr, nixbpe, disp, sign_flag, r1, r2를 가지고 명령을 실행.
		Execute(format, &current_addr, op, nixbpe, disp, sign_flag, r1, r2);

	}
	// 메모리 해제
	deleteESTabRoot(ESTabRoot);
	deleteLoadmap(LoadmapRoot);
}

// link와 load를 Pass1과 Pass2를 거쳐 진행한다.
void link_and_load(char parsedInstruction[][MAX_PARSED_NUM + 10]) {
	sizeOfBP = 0;
	memset(BPlist, -1, sizeof(BPlist));
	if (LinkingPass1(parsedInstruction)) {
		printf("Link pass1 error\n");
	}
	if (LinkingPass2(parsedInstruction)) {
		printf("Link pass2 error!\n");
	}
}

// Breakpoint와 관련한 명령어들.
void BreakPoint(int parameters[], char str[]) {

	// clear flag가 넘어오면 sizeOfBP를 0으로 만든다.
	if (parameters[0] == CLEAR_BP_FLAG) {
		sizeOfBP = 0;
		memset(BPlist, -1, sizeof(BPlist));
		printf("            [ok] clear all breakpoints\n");
	}
	// print flag가 넘어오면 list를 순서대로 출력한다.
	else if (parameters[0] == PRINT_BP_FLAG) {
		printf("            breakpoint\n");
		printf("            ----------\n");
		for (int i = 0; i < sizeOfBP; i++) {
			printf("            %0X\n", BPlist[i]);
		}
	}
	// 그것이 아니면 Breakpoint list에 추가한다.
	else {
		BPlist[sizeOfBP++] = parameters[0];
		printf("            [ok] create breakpoint %s\n", str);
	}
}

// Progaddr을 설정.
void setProgaddr(int parameters[]) {
	Progaddr = parameters[0];
}