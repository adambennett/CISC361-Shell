#ifndef wildcard_h
#define wildcard_h

#include "sh.h"

bool hasWildcards(char *commandline);						// Checks the entered commandline to see if there's any ? or *
char **expand(char **args, int argc);						// Expands all passed in args 
char *expandArgs(char *arg);								// Expands just one arg
int whichArgIsWild(char **args);							// Finds the wild argument's index within argsEx
int howManyNewArgs(char **args, int indexOfWildArg);

#endif //wildcard_h