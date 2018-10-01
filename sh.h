#ifndef sh_h
#define sh_h

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <glob.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "structs.h"
#include "util.h"
#include "environ.h"
#include "builtins.h"
#include "plumbing.h"
#include "execute.h"
#include "alias.h"
#include "main.h"
#include "wildcard.h"

int pid;
int sh( int argc, char **argv, char **envp);									//Essentially the main() function of this program
void commandSet(pathelement *pathlist, char *command, bool cont, bool print);	//Used with which, where and for finding commands
void commandFind(pathelement *pathlist, char *command, bool cont, bool print);	//Used with where
void printPathlist(pathelement *pathlist);										//Prints current PATH
int listCheck(char *dir);														//Checks if a directory is open-able
void listHelper(int q, char *owd, char **args);									//Used with list to print

#define PROMPTMAX 32
#define MAXARGS 10
#define MAXMEM 100
#define BUFFER 128
#define MAXTOK 512
#define WILDCARDS "*?"

#endif //sh_h