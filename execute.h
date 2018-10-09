#ifndef ex_h
#define ex_h

#include "sh.h"

int execute(char *cmd, char **argv, char **env, pid_t pid, int status, bool trigWild, bool bg);	//Where we fork()
int lineHandler(int *argc, char ***args, char *commandline);									//Commandline parser
void exec_command(char *command, char *commandlineCONST, char **args, char **env, 				//Executes commands from sh.c
				  pid_t pid, pathelement *pathlist, int status, bool trigWild, bool bg);

#endif //ex_h