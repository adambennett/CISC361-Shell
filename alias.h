#ifndef alias_h
#define alias_h

#include "sh.h"

bool isAlias(char *string, aliasEntry aliasList[], int value, int count);		// Checks to see if a string is an existing alias
int countAliasEntries(aliasEntry aliasList[], int count);						// Counts the amount of entries in the alias list
void print_aliases(aliasEntry aliasList[], int count);							// Prints the contents of the alias list to the console
int addAlias(char *string, char *command, aliasEntry aliasList[], int count);	// Intelligently adds new or existing alias to list of aliases
int proc_alias(aliasEntry aliasList[], int argc, char **args, int count);		// Called from sh.c when the user enters 'alias' into the shell
int morphAlias(char *command, aliasEntry aliasList[], int count);				// Call from sh.c if the user enters a command that is an alias

#endif //alias_h