#ifndef built_h
#define built_h

#include "sh.h"

int which(char *command, char **builtins, char *arg, int features, pathelement *pathlist);
void where(char **command, pathelement *pathlist, char **builtins, int features);
char **cd (char **args, char *pwd, char *owd, char *homedir, char **dirMem, int argc);
void list ( char *dir);
int prompter(char **args, char *prompt, int argc);
int hist(char **args, int mem, char **memory, int mems, int argc);
void kill_proc(char **args, int argc, char *prompt, char *buf, char *owd, char *pwd, char *prev, 
			char **dirMem, char ***memory, pathelement *pathlist, 
			char *commandlineCONST, char ***argsEx, char **envMem, char **returnPtr, char *memHelper,
			char *memHelper2, char *pathRtr, pid_t pid, int aliases, aliasEntry aliasList[]);


#endif //built_h