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
#include "environ.h"
#include "builtins.h"
#include "plumbing.h"
#include "execute.h"
#include "alias.h"
#include "main.h"
//#include "wildcard.h"

int pid;
int sh( int argc, char **argv, char **envp);
void commandSet(pathelement *pathlist, char *command, bool cont, bool print);
void commandFind(pathelement *pathlist, char *command, bool cont, bool print);
void printPathlist(pathelement *pathlist);
int listCheck(char *dir);
void listHelper(int q, char *owd, char **args);

#define PROMPTMAX 32
#define MAXARGS 10
#define MAXMEM 100
#define BUFFER 128
#define MAXTOK 512

#endif //sh_h