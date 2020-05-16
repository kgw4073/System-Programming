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
	temp->front = NULL;
	temp->rear = temp->front;
	temp->length = -1;
	temp->left = temp->right = NULL;
	return temp;
}
ESTab* insertESTab(ESTab* root, char ControlSection[], int CSAddr, int length) {
	//빈 노드
	if (root->length == -1) {
		strcpy(root->ControlSection, ControlSection);
		root->CSAddr = CSAddr;
		root->length = length;
		return root;
	}
	int res = strcmp(root->ControlSection, ControlSection);
	if (res > 0) {
		if (!root->left) {
			ESTab* temp = getNewESTabNode();
			root->left = temp;
		}
		return insertESTab(root->left, ControlSection, CSAddr, length);
	}
	else if (res < 0) {
		if (!root->right) {
			ESTab* temp = getNewESTabNode();
			root->right = temp;
		}
		return insertESTab(root->right, ControlSection, CSAddr, length);
	}
}

ExtSymbol* getNewExtSymbolNode() {
	ExtSymbol* temp = (ExtSymbol*)malloc(sizeof(ExtSymbol));
	temp->addr = -1;
	temp->Symbol[0] = '\0';
	temp->next = (ExtSymbol*)malloc(sizeof(ExtSymbol));
	return temp;
}
void insertTableNode(ESTab* root, char name[], int nodeAddr) {
	if (!root->rear) {
		root->front = getNewExtSymbolNode();
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

bool LinkingPass1(char filenames[][MAX_PARSED_NUM + 10]) {
	int CSADDR = Progaddr;
	ESTabRoot = getNewESTabNode();
	char line[1000];
	char* pLine;

	/*ESTabRoot = (ESTab*)malloc(sizeof(ESTab));
	ESTabRoot->front = NULL;
	ESTabRoot->rear = ESTabRoot->front;
	ESTabRoot->ControlSection[0] = '\0';*/
	
	for (int i = 1; ; i++) {
		if (filenames[i][0] == '\0') {
			break;
		}
		FILE* fp = fopen(filenames[i], "r");
		while (1) {
			ESTab* currentTableRoot;
			line[0] = '\0';
			fgets(line, 1000, fp);
			if (line[0] == 'E') break;
			if (line[0] == '.') continue;
			if (line[0] == 'H') {
				pLine = line + 1;
				char ControlSection[100], Temp[100];
				char Start[100], Length[100];
				sscanf(pLine, "%s %6s%6s", ControlSection, Start, Length);
				int CSAddr = strtol(Start, NULL, 16) + CSADDR;
				int length = strtol(Length, NULL, 16);
				ESTab* search = findESTab(ESTabRoot, ControlSection);
				// 가능
				if (search == NULL) {
					currentTableRoot = insertESTab(ESTabRoot, ControlSection, CSAddr, length);
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
					int nodeAddr = strtol(temp, NULL, 16) + CSADDR;
					insertTableNode(currentTableRoot, name, nodeAddr);
					pLine += 12;
				}
			}

			
		}
		printf("\n");
		fclose(fp);

	}
}

void LinkingPass2() {

}
void link_and_load(char parsedInstruction[][MAX_PARSED_NUM + 10]) {
	sizeOfBP = 0;
	memset(BPlist, -1, sizeof(BPlist));
	LinkingPass1(parsedInstruction);
	LinkingPass2();


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