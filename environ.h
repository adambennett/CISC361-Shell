#ifndef environ_h
#define environ_h

#include "sh.h"

void envprint(char **env, char **args, int argc, char **vars);
int envCheck(char **env, char **args, int argc);
char *envSet(char **args, char **env, struct pathelement *pathlist, int argc, char **vars, bool freeEnvp, bool freePath);
char *newEnvVar(char **env, char *name, char *value, char **vars);
void reinitEnv(char **env, bool freeEnvp);
int countEntries(char **array);

#endif //environ_h