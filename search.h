#ifndef search_h
#define search_h

#include "sh.h"

char *which(char *command, char **builtins, char *arg, int features, struct pathelement *pathlist, bool freePath);
char *quickwhich(char *cmd, struct pathelement *pathlist, bool freePath);
int where(char *command, struct pathelement *pathlist, char **builtins, int features, bool freePath);

#endif //search_h