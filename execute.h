#ifndef ex_h
#define ex_h

#include "sh.h"

int execute(char *cmd, char **argv, char **env, pid_t pid, int status, bool trigWild);
int lineHandler(int *argc, char ***args, char ***argv, char *commandline);
void exec_command(char *command, char *commandlineCONST, char **argsEx, char **env, pid_t pid, pathelement *pathlist, int status, bool trigWild);

#endif //ex_h