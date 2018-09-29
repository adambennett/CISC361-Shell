#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
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
#include "sh.h"


/** 
 * @brief Essentially the main() function of this program
 *
 * This is where the main processing during shell execution takes place. 
 * 			
 * @param argc   		Number of arguments passed into the shell when started
 * @param argv			String array of arguments passed into the shell program
 * @param envp			Environment variable
 */
int sh( int argc, char **argv, char **envp )
{
	char *builtIns[2048] = {"cd", "which", "exit", "where", "pwd", "history", "pid", "kill", "list", "printenv", "setenv", "alias", "prompt", "hist", "quit"};
	char *prompt = calloc(PROMPTMAX, sizeof(char));
	char *commandline = calloc(MAX_CANON, sizeof(char));
	char *commandlineCONST = calloc(MAX_CANON, sizeof(char));
	char *buf = calloc(MAX_CANON, sizeof(char));
	char *command, *pwd, *owd, *homedir, *prev, *tempHome;
	char **args = calloc(MAXARGS, sizeof(char*));
	char **argsEx = calloc(MAXARGS, sizeof(char*));
	char **memory = calloc(MAXMEM, sizeof(char*));
	char **dirMem = calloc(MAXARGS, sizeof(char*));
	char **envMem = calloc(MAXMEM, sizeof(char*));
	char *returnPtr = NULL;
	bool go = true;
	int uid, status = 1;
	int argsc, h = 0;
	int features = 15;						// number of built in functions
	int mem = 0;
	int mems = 0;
	struct passwd *password_entry;
	struct pathelement *pathlist;
	pid_t	pid = getpid();
	uid = getuid();
	password_entry = getpwuid(uid);         // get passwd info
	homedir = password_entry->pw_dir;		// Home directory to start out with

	 
	if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
	{
		perror("getcwd");
		exit(2);
	}
	owd = calloc(strlen(pwd) + 1, sizeof(char));
	prev = calloc(strlen(pwd) + 1, sizeof(char));
	dirMem[0] = malloc(sizeof(char*));
	dirMem[1] = malloc(sizeof(char*));
	memcpy(owd, pwd, strlen(pwd));
	memcpy(prev, owd, strlen(pwd));
	memcpy(dirMem[0], owd, sizeof(char));
	memcpy(dirMem[1], owd, sizeof(char));
	tempHome = malloc(sizeof(char));
	
	prompt[0] = ' '; prompt[1] = '\0';
	pathlist = get_path();					// Put PATH into a linked list
	pathlist->head = calloc(1, sizeof(struct pathelement));
	pathlist->head = pathlist;
	
	fillEnvMem(envMem, envp);
	headRef(pathlist);
	
	/*
	while (pathlist->next != NULL)
	{
		pathlist->next->head = pathlist->head;
		pathlist = pathlist->next;
	}
	*/
	
	while (go)
	{
		signal(SIGINT, sigintHandler);
		signal(SIGTSTP, signalSTPHandler);
		if (go) { fprintf(stderr, "%s[%s]>", prompt, owd); }
		while ((fgets(commandline, MAX_CANON, stdin) != NULL) && go) 
		{
			if (commandline == NULL) { printf("\n\n"); continue; }
			if (commandline[strlen(commandline) - 1] == '\n')
			{
				if (strlen(commandline) > 1) 
				{
					commandline[strlen(commandline) - 1] = 0; 	// replace newline with null
				} 
			}
			
			strcpy(commandlineCONST, commandline);
			int line = lineHandler(&argsc, &argsEx, &args, commandline);
			if (line == 1) 
			{
				if (strcmp(commandlineCONST, "\n") != 0)
				{
					memory[h] = calloc(strlen(commandlineCONST) + 1, sizeof(char));
					strcpy(memory[h], commandlineCONST);
					h++; mems++;
					if (mem < 10) { mem++; }
					if (mem > 10) { mem = 10; }
				}
			}
			
			//command = realloc(command, (size_t) (strlen(argsEx[0]) + 1) * sizeof(char));
			command = malloc(strlen(argsEx[0]) + 1);
			strcpy(command, argsEx[0]);
			
			
			// BUILT IN COMMANDS //
			if ((strcmp(command, "exit") == 0) || (strcmp(command, "EXIT") == 0) || (strcmp(command, "quit") == 0))
			{
				printf("Executing built-in exit\n");
				go = false;
			}
			
			else if ((strcmp(command, "which") == 0) || strcmp(command, "WHICH") == 0)
			{
				printf("Executing built-in which\n");
				pathlist = pathlist->head;
				refreshPath(pathlist);
				which(args[0], builtIns, args[1], features, pathlist);
				//printf("%s\n", command);
			}
			
			else if ((strcmp(command, "where") == 0) || strcmp(command, "WHERE") == 0)
			{
				printf("Executing built-in where\n");
				pathlist = pathlist->head;
				refreshPath(pathlist);
				where(args[0], pathlist, builtIns, features);
			}
			
			else if (strcmp(command, "cd") == 0)
			{
				printf("Executing built-in cd\n");
				dirMem = cd(args, pwd, owd, homedir, dirMem, argsc);
				prev = realloc(prev, (size_t) sizeof(char)*(strlen(dirMem[0]) + 1));
				owd = realloc(owd, (size_t) sizeof(char)*(strlen(dirMem[1]) + 1));
				strcpy(prev, dirMem[0]);
				strcpy(owd, dirMem[1]);
				free(pwd); 
				pwd = malloc(strlen(dirMem[1]) + 1); 
				strcpy(pwd, dirMem[1]);
			}
		
			else if ((strcmp(command, "pwd") == 0) || (strcmp(command, "PWD") == 0))
			{
				printf("Executing built-in pwd\n");
				printf("%s\n", owd);
			}
			
			else if ((strcmp(command, "prev") == 0) || (strcmp(command, "previous") == 0))
			{
				printf("Executing built-in prev\n");
				printf("%s\n", prev);
			}

			else if ((strcmp(command, "prompt") == 0) || (strcmp(command, "PROMPT") == 0))
			{
				printf("Executing built-in prompt\n");
				prompter(args, prompt, argsc);
			}
			
			else if ((strcmp(command, "pid") == 0) || (strcmp(command, "PID") == 0))
			{
				printf("Executing built-in pid\n");
				printf("pid = %jd\n", (intmax_t) pid);
			}
			
			else if ((strcmp(command, "history") == 0) || (strcmp(command, "HISTORY") == 0) || (strcmp(command, "hist") == 0))
			{
				printf("Executing built-in history\n");
				hist(args, mem, memory, mems, argsc);
			}
			
			else if ((strcmp(command, "list") == 0) || (strcmp(command, "LIST") == 0))
			{
				printf("Executing built-in list\n");
				listHelper(argsc, owd, args);
			}
			
			else if (strcmp(command, "printenv") == 0)
			{
				bool check = true;
				if (argsc > 1) { if (getenv(args[0]) == NULL) { check = false; } }
				if (check) 
				{ 
					printf("Executing built-in printenv\n");
					envprint(envp, args, argsc, envMem);
				}
			}
			
			else if (strcmp(command, "setenv") == 0)
			{
				printf("Executing built-in setenv\n");
				pathlist = pathlist->head;
				refreshPath(pathlist);
				returnPtr = envSet(args, envp, pathlist, argsc, envMem);
			}
			
			else if ((strcmp(command, "alias") == 0) || (strcmp(command, "ALIAS") == 0))
			{
				printf("Executing built-in alias\n");
			}
			
			else if ((strcmp(command, "kill") == 0) || (strcmp(command, "KILL") == 0) || (strcmp(command, "DESTROY") == 0))
			{
				printf("Executing built-in kill\n");
				kill_proc(args, argsc);
			}
			
			else if (strcmp(command, "\n") == 0) {}		
			else if (strcmp(command, "refreshpath") == 0) 
			{
				pathlist = pathlist->head;
				refreshPath(pathlist);
			}		

			else if (strcmp(command, "debugprint") == 0) 
			{
				arrayPrinter(envMem);
			}					
			// END BUILT IN COMMANDS
			
			else 
			{
				pathlist = pathlist->head;
				refreshPath(pathlist);
				exec_command(command, commandlineCONST, argsEx, envp, pid, pathlist, status); 
			}
			
			if (go) { argsc = 0; fprintf(stderr, "%s[%s]>", prompt, owd); }
			else { break; }
		}
	}
	plumber(prompt, buf, owd, pwd, prev, dirMem, args, &memory, pathlist, argsc, mems, commandlineCONST, tempHome, command, &argsEx, envMem, returnPtr);
	return 0;
} 