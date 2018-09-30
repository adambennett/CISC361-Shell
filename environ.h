#ifndef environ_h
#define environ_h

#include "sh.h"

void envprint(char **env, char **args, int argc, char **vars);
char *envSet(char **args, char **env, pathelement **pathlist, int argc, char **vars, char *pathRtr, bool clearedPath);
char *newEnvVar(char **env, char *name, char *value, char **vars);
int countEntries(char **array);
void fillEnvMem(char **envMem, char **envp);
void arrayPrinter(char **array);
void copyArray(char **to, char **from);
void copyArrayIndexed(char **to, char **from, int index);
char *get_path(pathelement **pathlist);
char *refreshPath(pathelement *pathlist);
void headRef(pathelement *pathlist);

#endif //environ_h