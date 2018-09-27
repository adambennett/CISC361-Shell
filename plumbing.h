#ifndef plumb_h
#define plumb_h

#include "sh.h"

void pathPlumber(struct pathelement *pathlist);
void plumber(char *prompt, char *commandline, char *buf, 
			 char *owd, char *pwd, char *prev, char **dirMem, 
			 char **args, char **memory, struct pathelement *pathlist, 
			 int q, int mems, char *commandlineCONST);


#endif //plumb_h