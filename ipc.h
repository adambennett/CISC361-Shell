#ifndef ipc_h
#define ipc_h

#include "sh.h"

bool isIPC(char *line);																						//Determines if line contains IPC operator
int parse_ipc_line(char** left, char** right, char* line);													//Determines IPC operator type and left/right commands
void perform_ipc(char* left, char* right, int ipc_type, char **envp, char **envMem, pathelement *pathlist); //Performs the actual IPC operation
void proc_command(char *commandline, char **envp, char **envMem, pathelement *pathlist);					//Runs builtin and external commands

#endif //ipc_h