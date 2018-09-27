#ifndef search_h
#define search_h

#include "sh.h"

char *which(char *command, char **builtins, char *arg, int features, struct pathelement *pathlist);
char *quickwhich(char *cmd, struct pathelement *pathlist);
int where(char *command, struct pathelement *pathlist, char **builtins, int features);

#endif //search_h