#ifndef sh_h
#define sh_h

// Libraries
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

// Headers
#include "structs.h"
#include "util.h"
#include "environ.h"
#include "builtins.h"
#include "plumbing.h"
#include "execute.h"
#include "alias.h"
#include "main.h"
#include "wildcard.h"
#include "redirection.h"
#include "ipc.h"

// Global Variables
pthread_mutex_t watchuser_lock;
char *prompt;																	// The prompt printed before [cwd]> before each user input
char **memory;																	// The saved command stack
bool firstUser;																	// Set to true until a user is added to the watchuser list						
bool bg;																		// Set to true while the current command is to be backgrounded	
int aliases;																	// Keeps track of # of aliases
int mem;																		// Used in history to keep track of how many commands to print
int mems;																		// Used with history to keep track of how many commands have been saved
int status;																		// Child status
aliasEntry aliasList[100];														// Holds the list of aliases
pthread_t tid1;																	// Watchuser thread
pid_t pid;																		// Parent process ID

// Sh Functions
int sh( int argc, char **argv, char **envp);									//Essentially the main() function of this program
void commandSet(pathelement *pathlist, char *command, bool cont, bool print);	//Used with which, where and for finding commands
void commandFind(pathelement *pathlist, char *command, bool cont, bool print);	//Used with where
void printPathlist(pathelement *pathlist);										//Prints current PATH
void printUsers(userList *usersHead);											//Prints watchuser list
void printMail(mailList *mailHead);												//Prints watchmail list
int countUsers(userList *usersHead);											//Counts watched users
int countMail(mailList *mailHead);												//Counts watched files
bool isUser(userList *usersHead, char *userName);								//Checks usersList for username
bool isMail(mailList *mailHead, char *fileName);								//Checks mailList for filename
int listCheck(char *dir);														//Checks if a directory is open-able
void listHelper(int q, char *owd, char **args);									//Used with list to print
int lastChar(const char *str);													//Check if last character in given string is '&'
void *watchuser(void *param);													//Called by the watchuser thread
void *watchmail(void *param);													//Called by the watchmail thread(s)
void addUser(char *userName, userList **usersHead);								//Adds a user to watched users list
void addMail(char *fileName, mailList **mailHead);								//Adds a file to watched files list
bool removeUser(char *userName, userList **head);								//Removes user from watched list
bool removeMail(char *fileName, mailList **head);								//Removes file from watched list
void proc_watchuser(int argsc, char **args, bool firstUser, pthread_t tid1);	//Called when user enters watchuser
void proc_watchmail(int argsc, char **args);									//Called when user enters watchmail

// Global Constants
#define PROMPTMAX 32
#define MAXARGS 10
#define MAXMEM 100
#define BUFFER 128
#define MAXTOK 512
#define WILDCARDS "*?"

#endif //sh_h