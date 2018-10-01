#ifndef ex_h
#define ex_h

#include "sh.h"

int execute(char *cmd, char **argv, char **env, pid_t pid, int status, bool trigWild);	//Where we fork()
int lineHandler(int *argc, char ***args, char ***argv, char *commandline);				//Commandline parser
void exec_command(char *command, char *commandlineCONST, char **argsEx, char **env, 	//Executes commands from sh.c
				  pid_t pid, pathelement *pathlist, int status, bool trigWild);

#endif //ex_h