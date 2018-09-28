#ifndef environ_h
#define environ_h

#include "sh.h"

void envprint(char **env, char **args, int argc, char **vars);
int envCheck(char **env, char **args, int argc);
char *envSet(char **args, char **env, struct pathelement *pathlist, int argc, char **vars);
char *newEnvVar(char **env, char *name, char *value, char **vars);
int countEntries(char **array);

#endif //environ_h