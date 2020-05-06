#ifndef _LINK_LOADER_H_
#define _LINK_LOADER_H_

int Progaddr;
int BPlist[1000];
int sizeOfBP;


extern void link_and_load(char parsedInstruction[][MAX_PARSED_NUM + 10]);
extern void BreakPoint(int parameters[], char str[]);
extern void setProgaddr(int parameters[]);

#endif