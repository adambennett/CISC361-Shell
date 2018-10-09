#ifndef built_h
#define built_h

#include "sh.h"

int which(char *command, char **builtins, char *arg, int features, pathelement *pathlist);		//which helper function
void where(char **command, pathelement *pathlist, char **builtins, int features);				//where helper function
void changeDirectory(char **envp, char **args, int argc, char **envMem);						//cd helper function		
void list ( char *dir);																			//list command helper function
int prompter(char **args, char *prompt, int argc);												//prompt helper function
int hist(char **args, int mem, char **memory, int mems, int argc);								//history helper function
void kill_proc(int argc, char *prompt, 			//kill helper function
			char ***memory, pathelement *pathlist, 
			char *commandlineCONST, char ***args, char **envMem, char **returnPtr, char *memHelper,
			char *memHelper2, char *pathRtr, pid_t pid, int aliases, aliasEntry aliasList[]);
void newUser(char *userName, userList *usersHead, userList *usersTail);
void deleteUser(userList *users, char *userName, userList *usersHead, userList *usersTail);

#endif //built_h