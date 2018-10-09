#ifndef plumb_h
#define plumb_h

#include "sh.h"

void pathPlumber(pathelement *pathlist);				//Frees a linked list
void arrayPlumber(char **array, int size);				//Frees a char ** array
void plumber(char *prompt, char ***memory, pathelement *pathlist, char *commandlineCONST, 
			char ***args, char **envMem, char **returnPtr, char *memHelper,char *memHelper2, 
			char *pathRtr, bool checker, int aliases, aliasEntry aliasList[]);					//Memory leak helper function - attempt to free everything else


#endif //plumb_h