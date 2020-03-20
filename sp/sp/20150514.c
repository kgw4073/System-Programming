#include "20150514.h"
#include "command.h"
#include "BasicCommand.h"

bool quit_flag = false;
Trie* root = NULL;

void init() {
	char basicInstruction[17][100] = {
	"h", "help", "d", "dir", "q", "quit", "hi", "history", "du", "dump",
	"e", "edit", "f", "fill", "reset", "opcode", "opcodelist"
	};
	root = getNewTrieNode();
	for (int i = 0; i < 17; i++) {
		insertTrie(root, basicInstruction[i]);
	}

	head_of_command_queue = (command_list*)malloc(sizeof(command_list));
	tail_of_command_queue = head_of_command_queue;
}
int main() {


	init();

	while (1) {
		if (quit_flag) break;
		printf("sicsim> ");

		char instruction[MAX_COMMAND_SIZE];
		memset(instruction, 0, sizeof(instruction));
		fgets(instruction, MAX_COMMAND_SIZE, stdin);
		int len = (int)strlen(instruction);
		instruction[len - 1] = '\0';
		if (!command_parsing(instruction)) {
			fprintf(stderr, "Invalid command! : your command is currently not \
installed. To check command list, enter \"help\"\n");
		}

	}
	
	return 0;
}