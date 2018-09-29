#ifndef search_h
#define search_h

#include "sh.h"

int which(char *command, char **builtins, char *arg, int features, struct pathelement *pathlist);
//char *quickwhich(char *cmd, struct pathelement *pathlist);
int where(char *command, struct pathelement *pathlist, char **builtins, int features);
void commandSet(struct pathelement *pathlist, char *command, bool cont);
void printPathlist(struct pathelement *pathlist);

#endif //search_h