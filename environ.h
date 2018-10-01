#ifndef environ_h
#define environ_h

#include "sh.h"

void envprint(char **env, char **args, int argc, char **vars);			//printenv helper function
char *envSet(char **args, char **env, pathelement **pathlist, 			//setenv helper function
			 int argc, char **vars, char *pathRtr, bool clearedPath);
char *newEnvVar(char **env, char *name, char *value, char **vars);		//Add/update enviroment variable
char *get_path(pathelement **pathlist);									//Puts PATH into a linked list
char *refreshPath(pathelement *pathlist);								//Refreshes the pathlist
void headRef(pathelement *pathlist);									//Fixes pathlist heads

#endif //environ_h