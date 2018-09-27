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

int sh( int argc, char **argv, char **envp )
{
	char *builtIns[2048] = {"cd", "which", "exit", "where", "pwd", "history", "pid", "kill", "list", "printenv", "setenv", "alias", "prompt", "hist", "quit"};
	char *prompt = calloc(PROMPTMAX, sizeof(char));
	char *commandline = calloc(MAX_CANON, sizeof(char));
	char *commandlineCONST = calloc(MAX_CANON, sizeof(char));
	char *buf = calloc(MAX_CANON, sizeof(char));
	char *command, *arg, *commandpath, *p, *pwd, *owd, *homedir, *prev;
	char **args = calloc(MAXARGS, sizeof(char*));
	char **argsEx = calloc(MAXARGS, sizeof(char*));
	char **memory = calloc(MAXMEM, sizeof(char*));
	char **dirMem = calloc(MAXARGS, sizeof(char*));
	char **env = envp;
	bool go = true;
	int uid, i, status, argsct = 1;
	int q, j, h, e = 0;
	int features = 15;						// number of built in functions
	int mem = 0;
	int mems = 0;
	size_t n = 1;
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
	
	prompt[0] = ' '; prompt[1] = '\0';
	pathlist = get_path();					// Put PATH into a linked list

	while (go)
	{
		signal(SIGINT, sigintHandler);
		signal(SIGTSTP, signalSTPHandler);
		if (go) { fprintf(stderr, "%s[%s]>", prompt, owd); }
		while ((fgets(commandline, MAX_CANON, stdin) != NULL) && go) 
		{
			
			if (commandline[strlen(commandline) - 1] == '\n')
			{
				if (strlen(commandline) > 1) 
				{
					commandline[strlen(commandline) - 1] = 0; 	// replace newline with null
				} 
			}
			
			strcpy(commandlineCONST, commandline);
			int line = lineHandler(&q, &argsEx, &args, commandline);
			if (line == 1) 
			{
				memory[h] = calloc(strlen(commandlineCONST) + 1, sizeof(char));
				strcpy(memory[h], commandlineCONST);
				h++; mems++;
				if (mem < 10) { mem++; }
				if (mem > 10) { mem = 10; }
			}
			
			command = realloc(command, (size_t) (strlen(argsEx[0]) + 1) * sizeof(char));
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
				char *foundCommand = which(args[0], builtIns, args[1], features, pathlist);
				printf("%s\n", foundCommand);
				pathlist = get_path();
			}
			
			else if ((strcmp(command, "where") == 0) || strcmp(command, "WHERE") == 0)
			{
				printf("Executing built-in where\n");
				where(args[0], pathlist, builtIns, features);
				pathlist = get_path();
			}
			
			else if (strcmp(command, "cd") == 0)
			{
				printf("Executing built-in cd\n");
				dirMem = cd(args, pwd, owd, homedir, dirMem, q);
				prev = realloc(prev, (size_t) sizeof(char)*(strlen(dirMem[0]) + 1));
				owd = realloc(owd, (size_t) sizeof(char)*(strlen(dirMem[1]) + 1));
				strcpy(prev, dirMem[0]);
				strcpy(owd, dirMem[1]);
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
				printf("Executing built-in prompt (debug)\n");
				prompt = prompter(args, prompt, q);
			}
			
			else if ((strcmp(command, "pid") == 0) || (strcmp(command, "PID") == 0))
			{
				printf("Executing built-in pid\n");
				printf("pid = %jd\n", (intmax_t) pid);
			}
			
			else if ((strcmp(command, "history") == 0) || (strcmp(command, "HISTORY") == 0) || (strcmp(command, "hist") == 0))
			{
				printf("Executing built-in history\n");
				mem = hist(command, args, mem, memory, mems, q);
			}
			
			else if ((strcmp(command, "list") == 0) || (strcmp(command, "LIST") == 0))
			{
				printf("Executing built-in list\n");
				listHelper(q, owd, args);
			}
			
			else if (strcmp(command, "printenv") == 0)
			{
				int check = 2;
				check = envCheck(env, args);
				if (check > 0) { printf("Executing built-in printenv\n"); }
				pathlist = get_path();
				envprint(env, args, q);
			}
			
			else if (strcmp(command, "setenv") == 0)
			{
				printf("Executing built-in setenv\n");
				pathlist = get_path();
				envSet(args, env, pathlist, q);
				char *tempHome = findName(env, "HOME");
				strcpy(homedir, tempHome);
				homedir = strtok(homedir, "=");
				homedir = strtok(NULL, "=");
			}
			
			else if ((strcmp(command, "alias") == 0) || (strcmp(command, "ALIAS") == 0))
			{
				printf("Executing built-in alias\n");
			}
			
			else if ((strcmp(command, "kill") == 0) || (strcmp(command, "KILL") == 0) || (strcmp(command, "DESTROY") == 0))
			{
				printf("Executing built-in kill\n");
				kill_proc(args, q);
			}
			
			else if (strcmp(command, "\n") == 0)
			{
				
			}		

			else if (strcmp(command, "debug") == 0)
			{
				
			}					
			// END BUILT IN COMMANDS
			else
			{
				if( (command[0] == '/') || ((command[0] == '.') && ((command[1] == '/') ||(command[1] == '.') && (command[2] == '/'))))
				{
					execute(argsEx[0], argsEx, env, pid);
				}
				
				else
				{
					char *newCmd = quickwhich(command, pathlist);
					if (newCmd != NULL)
					{
						//argsEx[0] = realloc(argsEx[0], (size_t) (strlen(newCmd) + 1) * sizeof(char));
						//strcpy(argsEx[0], newCmd);
						*argsEx[0] = *newCmd;
						execute(newCmd, argsEx, env, pid);
					}
					else
					{
						printf("%s: Command not found.\n", commandlineCONST);
					}
				}
			}
		
			if (go) 
			{ 
				q = 0; 
				fprintf(stderr, "\n%s[%s]>", prompt, owd); 
			}
			else { break; }
		}
	}
	plumber(prompt, commandline, buf, owd, pwd, prev, dirMem, args, memory, pathlist, q, mems, commandlineCONST);
	return 0;
} 


int execute(char *cmd, char **argv, char **env, pid_t pid)
{

	pid = fork();
    int child_status;

    if(pid == 0) //** Executed in child process
	{                        

        execve(cmd, argv, env);
        
        // Exec commands only return if there's an error
        perror("Error in execute()");        
        
        // We exit since the process image will be replaced with itself here and
        // we will need to enter "exit" twice to truely exit.
        exit(0);        

    } 
	else if(pid > 0) //** Executed in parent process
	{                
            // TODO: add signal handler for SIGCHLD
            if(waitpid(pid, &child_status, 0) == -1)
			{
                perror("Error in waitpid");
            }
			if(WEXITSTATUS(child_status) != 0)
			{
				printf("Exited with code: %d\n", WEXITSTATUS(child_status)); // Print out the exit status if it is non-zero
			}
    } 
	else  //** Didn't fork properly 
	{
        perror("Fork failed\n");
    }

    return child_status;
}

int lineHandler(int *q, char ***args, char ***argv, char *commandline)
{
	int len = strlen(commandline);

	//## Tokenize the command into the argv array
	char* ptr = NULL;
	char* token = strtok_r(commandline, " ", &ptr);


	// If the line is blank, the first token will be the null string.
	if(token == '\0') { return 0; }       

	*args[0] = token;        // argv[0] is the command name

	for(int i = 1; token != NULL && i < MAXTOK; i++)
	{
		token = strtok_r(NULL, " \t", &ptr);
		(*args)[i] = token;
		(*argv)[i-1] = token;
		*q = i;
	}

	return 1;
}

