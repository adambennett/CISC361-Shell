#ifndef environ_h
#define environ_h

#include "sh.h"

void envprint(char **env, char **args, int argc);
int envCheck(char **env, char **args);
void envSet(char **args, char **env, struct pathelement *pathlist, int argc);
char *findName(char **envp, char *name);
void newEnvVar(char **env, char *name, char *value);
char **reinitEnv(char **env);
int countEntries(char **array);

#endif //environ_h