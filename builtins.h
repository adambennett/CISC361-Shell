#ifndef built_h
#define built_h

#include "sh.h"

int which(char *command, char **builtins, char *arg, int features, pathelement *pathlist);		//which helper function
void where(char **command, pathelement *pathlist, char **builtins, int features);				//where helper function
char **cd (char **args, char *pwd, char *owd, char *homedir, char **dirMem, int argc);			//cd helper function
void list ( char *dir);																			//list command helper function
int prompter(char **args, char *prompt, int argc);												//prompt helper function
int hist(char **args, int mem, char **memory, int mems, int argc);								//history helper function
void kill_proc(int argc, char *prompt, char *owd, char *pwd, char *prev, 			//kill helper function
			char **dirMem, char ***memory, pathelement *pathlist, 
			char *commandlineCONST, char ***args, char **envMem, char **returnPtr, char *memHelper,
			char *memHelper2, char *pathRtr, pid_t pid, int aliases, aliasEntry aliasList[]);

#endif //built_h