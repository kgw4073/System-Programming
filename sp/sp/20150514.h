
#ifndef _MAIN_HEADER_H
#define _MAIN_HEADER_H
#define MAX_COMMAND_SIZE 256
#define MAX_MEMORY_CAPACITY (1 << 20)
#define bool int
#define true 1
#define false 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>


char vMemory[MAX_MEMORY_CAPACITY];

extern bool quit_flag;
struct dumped {
	int last_address;
};
extern void init();
#endif
