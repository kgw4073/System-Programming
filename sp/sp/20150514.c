#include "20150514.h"
#include "command.h"
#include "BasicCommand.h"
bool quit_flag = false;
int main() {
	head_of_command_queue = NULL;
	tail_of_command_queue = head_of_command_queue;
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