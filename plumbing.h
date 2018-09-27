#ifndef plumb_h
#define plumb_h

#include "sh.h"

void pathPlumber(struct pathelement *pathlist);
void arrayPlumber(char **array, int size);
void ptrArrayPlumber(char ***array);
void plumber(char *prompt, char *commandline, char *buf, 
			 char *owd, char *pwd, char *prev, char **dirMem, 
			 char **args, char ***memory, struct pathelement *pathlist, 
			 int argc, int mems, char *commandlineCONST, char *tempHome,
			 char *command, char ***argsEx);


#endif //plumb_h