#include "sh.h"

int execute(char *cmd, char **argv, char **env, pid_t pid, int status)
{

	pid = fork();
    int child_status = status;

    if(pid == 0) //** Executed in child process
	{                        
		char temp[2046];
		strcpy(temp, argv[0]);
		for (int i = 1; argv[i] != NULL; i++) { strcat(temp, " "); strcat(temp, argv[i]); }
		printf("Executing %s\n", temp);
		printf("That came from execute() inside execute.c\n\n");
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

void exec_command(char *command, char *commandlineCONST, char **argsEx, char **env, pid_t pid, pathelement *pathlist, int status)
{
	// Doesn't handle ./ or ../ ??
	if( (command[0] == '/') || ((command[0] == '.') && ((command[1] == '/') ||((command[1] == '.') && (command[2] == '/')))))
	{
		if (strstr(command, ".sh") == NULL) 
		{ 
			execute(argsEx[0], argsEx, env, pid, status); 	// Check for access
		}	
		
		// This lets the shell attempt to execute .sh files if you give it one, however
		// .sh files only seem to run commands through the parent shell so this line
		// essentially just improves user experience theoretically
		else { execl("/bin/sh", "sh", "-c", command, (char *) 0); }	
	}
	
	else
	{
		commandSet(pathlist, command, false, false);
		if (command != NULL)
		{ 
			printf("Executing %s\n", commandlineCONST);
			printf("That came from exec_command() inside execute.c\n\n");
			execute(command, argsEx, env, pid, status);
		}
		else { printf("%s: Command not found.\n", commandlineCONST); }
	}
}