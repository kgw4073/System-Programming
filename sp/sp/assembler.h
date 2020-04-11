#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_
#include "20150514.h"
#include "command.h"



extern RETURN_CODE pass1(char filename[MAX_PARSED_NUM + 10]);
extern RETURN_CODE pass2(char filename[MAX_PARSED_NUM + 10]);
extern void doAssemble(char parsedInstruction[][MAX_PARSED_NUM + 10]);


#endif