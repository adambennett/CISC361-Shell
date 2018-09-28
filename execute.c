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

int execute(char *cmd, char **argv, char **env, pid_t pid, int status)
{

	pid = fork();
    int child_status = status;

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

int lineHandler(int *argc, char ***args, char ***argv, char *commandline)
{
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
		*argc = i;
	}

	return 1;
}

void exec_command(char *command, char *commandlineCONST, char **argsEx, char **env, pid_t pid, struct pathelement *pathlist, int status)
{
	//char *newCmd;
	if( (command[0] == '/') || ((command[0] == '.') && ((command[1] == '/') ||((command[1] == '.') && (command[2] == '/')))))
	{
		if (strstr(command, ".sh") == NULL) { execute(argsEx[0], argsEx, env, pid, status); }
		else { execl("/bin/sh", "sh", "-c", command, (char *) 0); }
	}
	
	else
	{
		command = quickwhich(command, pathlist);
		if (command != NULL)
		{
			//argsEx[0] = realloc(argsEx[0], (size_t) (strlen(newCmd) + 1) * sizeof(char));
			//strcpy(argsEx[0], newCmd);
			strcpy(argsEx[0], command);
			//*argsEx[0] = *newCmd;
			execute(command, argsEx, env, pid, status);
			//free(newCmd);
		}
		else
		{
			printf("%s: Command not found.\n", commandlineCONST);
			//free(newCmd);
		}
	}
}