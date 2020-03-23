
#ifndef _MAIN_HEADER_H
#define _MAIN_HEADER_H
#define MAX_COMMAND_SIZE 256
#define MAX_MEMORY_LINE (1 << 16)
#define MAX_BYTES_LINE (1 << 4)
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


unsigned char vMemory[MAX_MEMORY_LINE * MAX_BYTES_LINE];

extern bool quit_flag;
struct dumped {
	int last_address;
};
extern void init();
#endif
