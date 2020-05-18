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
//typedef struct ESTab {
//	char ControlSection[100];
//	int CSAddr, length;
//	struct ESTab* left, * right;
//} ESTab;
ESTab* ESTabRoot = NULL;

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
ESTab* getNewESTabNode() {
	ESTab* temp = (ESTab*)malloc(sizeof(ESTab));
	temp->ControlSection[0] = '\0';
	temp->CSAddr = 0;
	temp->length = -1;
	temp->left = temp->right = NULL;
	return temp;
}
void insertESTab(ESTab* root, char ControlSection[], int CSAddr, int length) {
	//빈 노드
	if (root->length == -1) {
		strcpy(root->ControlSection, ControlSection);
		root->CSAddr = CSAddr;
		root->length = length;
		return;
	}
	int res = strcmp(root->ControlSection, ControlSection);
	if (res > 0) {
		if (!root->left) {
			ESTab* temp = getNewESTabNode();
			root->left = temp;
		}
		insertESTab(root->left, ControlSection, CSAddr, length);
	}
	else if (res < 0) {
		if (!root->right) {
			ESTab* temp = getNewESTabNode();
			root->right = temp;
		}
		insertESTab(root->right, ControlSection, CSAddr, length);
	}
}


//void insertTableNode(ESTab* root, char name[], int nodeAddr) {
//	if (!root->rear) {
//		root->front = getNewExtSymbolNode();
//		strcpy(root->front->Symbol, name);
//		root->front->addr = nodeAddr;
//		root->rear = root->front->next;
//	}
//	else {
//		strcpy(root->rear->Symbol, name);
//		root->rear->addr = nodeAddr;
//		root->rear->next = getNewExtSymbolNode();
//		root->rear = root->rear->next;
//
//	}
//
//}

bool LinkingPass1(char filenames[][MAX_PARSED_NUM + 10]) {
	int CSADDR_BASE = Progaddr;
	ESTabRoot = getNewESTabNode();
	char line[1000];
	char* pLine;
	int CurrentSectionLength;

	for (int i = 1; ; i++) {
		if (filenames[i][0] == '\0') {
			break;
		}
		FILE* fp = fopen(filenames[i], "r");
		while (1) {
			line[0] = '\0';
			fgets(line, 1000, fp);
			if (line[0] == 'E') break;
			if (line[0] == '.') continue;
			if (line[0] == 'H') {
				pLine = line + 1;
				char ControlSection[100], Temp[100];
				char Start[100], Length[100];
				sscanf(pLine, "%s %6s%6s", ControlSection, Start, Length);
				int CSAddr = strtol(Start, NULL, 16) + CSADDR_BASE;
				CurrentSectionLength = strtol(Length, NULL, 16);
				
				ESTab* search = findESTab(ESTabRoot, ControlSection);
				// 가능
				if (search == NULL) {
					insertESTab(ESTabRoot, ControlSection, CSAddr, CurrentSectionLength);
				}
				// 불가능
				else return true;
			}
			else if (line[0] == 'D') {
				pLine = line + 1;
				while (1) {
					char name[100], temp[100];
					name[0] = '\0', temp[0] = '\0';
					sscanf(pLine, "%s %6s", name, temp);
					if (name[0] == '\0' || temp[0] == '\0') break;
					int nodeAddr = strtol(temp, NULL, 16) + CSADDR_BASE;
					insertESTab(ESTabRoot, name, nodeAddr, 0);
					pLine += 12;
				}
			}
		}
		CSADDR_BASE += CurrentSectionLength;
		fclose(fp);
	}
}

// Performs the actual loading, relocation, and linking of the program.

bool LinkingPass2(char filenames[][MAX_PARSED_NUM + 10]) {
	int CSADDR_BASE = Progaddr;
	int EXECADDR = Progaddr;
	
	int Relocations[1000];
	char line[1000];
	char* pLine;
	int CurrentSectionLength;
	char name[100];
	char ControlSection[100], Temp[100];
	char Start[100], Length[100];
	int index, CSAddr, value;
	int location, length;

	for (int i = 1; ; i++) {
		if (filenames[i][0] == '\0') {
			break;
		}
		FILE* fp = fopen(filenames[i], "r");
		while (1) {
			ESTab* currentTableRoot;
			memset(line, 0, sizeof(line));

			fgets(line, 1000, fp);
			if (line[0] == 'E') break;
			if (line[0] == '.') continue;
			if (line[0] == 'H') {
				Relocations[1] = CSADDR_BASE;
				continue;
				/*pLine = line + 1;

				sscanf(pLine, "%s %6s%6s", ControlSection, Start, Length);
				CSAddr = strtol(Start, NULL, 16) + CSADDR_BASE;
				CurrentSectionLength = strtol(Length, NULL, 16);
				
				ESTab* search = findESTab(ESTabRoot, ControlSection);
				insertESTab(ESTabRoot, ControlSection, CSAddr, CurrentSectionLength);*/
			}
			else if (line[0] == 'D') {
				continue;
				/*pLine = line + 1;
				while (1) {
					char name[100], temp[100];
					name[0] = '\0', temp[0] = '\0';
					sscanf(pLine, "%s %6s", name, temp);
					if (name[0] == '\0' || temp[0] == '\0') break;
					int nodeAddr = strtol(temp, NULL, 16) + CSADDR_BASE;
					insertESTab(currentTableRoot, name, nodeAddr, 0);
					pLine += 12;
				}*/
			}
			else if (line[0] == 'R') {
				pLine = line + 1;
				while (1) {
					Temp[0] = '\0';
					sscanf(pLine, "%2s%6s", Temp, name);
					if (Temp[0] == '\0') break;
					index = strtol(Temp, NULL, 10);
					pLine += 8;

					ESTab* search = findESTab(ESTabRoot, name);
					if (search == NULL) {
						return true;
					}
					Relocations[index] = search->RELOC_ADDR;

				}
			}
			// Text Record;
			else if (line[0] == 'T') {
				pLine = line + 1;
				
				sscanf(pLine, "%6s%2s", Start, Length);
				location = strtol(Start, NULL, 16) + CSADDR_BASE;
				length = strtol(Length, NULL, 16);
				pLine += 8;
				for (int k = 0; k < length; k++) {
					sscanf(pLine, "%2s", Temp);
					value = strtol(Temp, NULL, 16);
					vMemory[location] = (unsigned char)value;
				}
			}
			// Modification Record
			else if (line[0] == 'M') {
				pLine = line + 1;
				sscanf(pLine, "%6s%2s", Start, Length);
				location = strtol(Start, NULL, 16) + CSADDR_BASE;
				length = strtol(Length, NULL, 16);
				pLine += 8;
				if (length == 5) {
					int target = (int)vMemory[location + 2] + 256 * (int)vMemory[location + 1] + 256 * 256 * (int)vMemory[location];
					if (pLine[0] == '+') {
						pLine++;
						sscanf(pLine, "%2d", &index);
						target += Relocations[index];
					}
					else if (pLine[1] == '-') {
						pLine++;
						sscanf(pLine, "%2d", &index);
						target -= Relocations[index];
					}
					unsigned char* pMemory = vMemory + location;
					for (int j = 1; j <= 20; j++) {
						*(pMemory + (j + 3) / 8) |= (target & (1 << (20 - j)));
					}
				}
				else if (length == 6) {
					int target = (int)vMemory[location + 2] + 256 * (int)vMemory[location + 1] + 256 * 256 * (int)vMemory[location];
					if (pLine[0] == '+') {
						pLine++;
						sscanf(pLine, "%2d", &index);
						target += Relocations[index];
					}
					else if (pLine[1] == '-') {
						pLine++;
						sscanf(pLine, "%2d", &index);
						target -= Relocations[index];
					}
					unsigned char* pMemory = vMemory + location;
					for (int j = 1; j <= 24; j++) {
						*(pMemory + (j - 1) / 8) |= (target & (1 << (20 - j)));
					}
				}
			}
		}
		CSADDR_BASE += CurrentSectionLength;
		fclose(fp);
	}

}
void link_and_load(char parsedInstruction[][MAX_PARSED_NUM + 10]) {
	sizeOfBP = 0;
	memset(BPlist, -1, sizeof(BPlist));
	if (LinkingPass1(parsedInstruction) == false) {
		if (LinkingPass2(parsedInstruction) == false) {

		}
	}
	


}
void BreakPoint(int parameters[], char str[]) {
	if (parameters[0] == CLEAR_BP_FLAG) {
		sizeOfBP = 0;
		memset(BPlist, -1, sizeof(BPlist));
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