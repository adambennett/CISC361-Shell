#ifndef ipc_h
#define ipc_h

#include "sh.h"

bool isIPC(char *line);
int parse_ipc_line(char** left, char** right, char* line);
void perform_ipc(char* left, char* right, int ipc_type, char **envp, char **envMem, pathelement *pathlist);
void proc_command(char *commandline, char **envp, char **envMem, pathelement *pathlist);

#endif //ipc_h