#ifndef plumb_h
#define plumb_h

#include "sh.h"

void pathPlumber(pathelement *pathlist);
void arrayPlumber(char **array, int size);
void plumber(char *prompt, char *buf, char *owd, char *pwd, char *prev, 
			char **dirMem, char **args, char ***memory, pathelement *pathlist, 
			char *commandlineCONST, char ***argsEx, char **envMem, char **returnPtr, char *memHelper,
			char *memHelper2, char *pathRtr, bool checker, int aliases, aliasEntry aliasList[]);


#endif //plumb_h