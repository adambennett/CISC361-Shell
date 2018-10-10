#ifndef sh_h
#define sh_h

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <limits.h>
#include <pthread.h>
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
#include <utmpx.h>

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
pthread_mutex_t watchuser_lock;
char *prompt;
int sh( int argc, char **argv, char **envp);									//Essentially the main() function of this program
void commandSet(pathelement *pathlist, char *command, bool cont, bool print);	//Used with which, where and for finding commands
void commandFind(pathelement *pathlist, char *command, bool cont, bool print);	//Used with where
void printPathlist(pathelement *pathlist);										//Prints current PATH
void printUsers(userList *usersHead);											//Prints watchuser list
int countUsers(userList *usersHead);											//
bool isUser(userList *usersHead, char *userName);								//
int listCheck(char *dir);														//Checks if a directory is open-able
void listHelper(int q, char *owd, char **args);									//Used with list to print
int lastChar(const char *str);
void sig_child_handler(int signal);
void *watchuser(void *param);
void addUser(char *userName, userList **usersHead, userList **usersTail);
bool removeUser(char *userName, userList **head);

#define PROMPTMAX 32
#define MAXARGS 10
#define MAXMEM 100
#define BUFFER 128
#define MAXTOK 512
#define WILDCARDS "*?"

#endif //sh_h