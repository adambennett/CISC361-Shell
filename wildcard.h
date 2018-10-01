#ifndef wildcard_h
#define wildcard_h

#include "sh.h"

bool hasWildcards(char *commandline);			// Checks the entered commandline to see if there's any ? or *
char **expand(char **args, int argc);			// Expands all passed in args 
char *expandArgs(char *arg);					// Expands just one arg

#endif //wildcard_h