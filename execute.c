#include "sh.h"

/** 
 * @brief Where we fork()
 *
 * This function is called from exec_command to simplify program flow.
 * When this is called it forks the parent process and runs the command passed in.
 * This process only happens when the user enters a command that is not built into the program.
 * If this function is being called, command has already been fully processed, and is most likely
 * running as an absolute path at this point. 
 * 			
 * @param cmd   		The command to be executed
 * @param argv			String array of arguments passed into the shell program
 * @param env			Environment variable array
 * @param pid			Parent process ID
 * @param status		Helps keeps track of child status so it is reachable from sh.c
 * @param trigWild		Used to make sure the print output doesn't look ridiculous when you trigger wildcard handling
 * @param bg			True if the process to be executed should be done in the background, false otherwise
 * @param redir			True if the parent has run a redirection command before calling this function, false otherwise
 *
 * @return Returns the status of the child process after waitpid() completes in the parent.
 */
int execute(char *cmd, char **argv, char **env, pid_t pid, int status, bool trigWild, bool bg, bool redir)
{

	if ((pid = fork()) < 0) { perror("fork"); } 
    int child_status = status;

	// Child process
    if(pid == 0) 
	{                        
		char temp[2046];
		strcpy(temp, argv[0]);
		
		// If the user enter a wildcard command, we simply skip printing the arguments with the command
		if (!trigWild) { for (int i = 1; argv[i] != NULL; i++) { strcat(temp, " "); strcat(temp, argv[i]); } }
		if (!redir) { printf("Executing %s\n", temp); }
        execve(cmd, argv, env);	//already test for access in parent function
        
        // Exec commands only return if there's an error
        perror("Error in execute()");     
        exit(127);        

    } 
	
	// Parent Process
	else if(pid > 0) 
	{               
		if (bg) 
		{
			if(waitpid(pid, &child_status, WNOHANG) == -1){ perror("Backgrounding error"); }
		}
		else
		{
			if(waitpid(pid, &child_status, 0) == -1) 
			{ 
				//perror("Error in waitpid"); 	// Always throws this because we are ignoring SIGCHLD
			}
		}
			// Throw error if waitpid() returns -1
            
			
			// Print out the exit status if it is non-zero
			//if(WEXITSTATUS(child_status) != 0) { printf("Exited with code: %d\n", WEXITSTATUS(child_status));  }
			if (bg && !redir) { printf("Backgrounded job: %s\n", cmd); }
    } 
	
    return child_status;
}

/** 
 * @brief Commandline parser
 *
 * This function is called early on in the program loop from sh.c
 * When called, the commandline is passed in and parsed so that
 * args contains command, arg1, arg2, ..., argN, NULL;
 * argv contains arg1, arg2, ... argN, NULL;
 *
 * NOTE: This function modifies the commandline string with strtok()
 * 			
 * @param argc   		Number of arguments passed into the shell when started (modified by this function)
 * @param args			Array to be filled with all strings from the commandline
 * @param commandline	String that contains the users input from the sh.c fgets() loop
 *
 * @return Returns 1 if the line is parsed properly. Returns 0 if you try to parse a blank line.
 */
int lineHandler(int *argc, char ***args, char *commandline)
{
	// Create a reference pointer to use with strtok_r()
	char* ptr = NULL;
	
	// Being tokenizing the line
	char* token = strtok_r(commandline, " ", &ptr);

	// Handle blank lines
	if(token == '\0') { return 0; }       

	// args[0] is the command name
	*args[0] = token;        

	// Now loop to save the rest of the arguments
	for(int i = 1; token != NULL && i < MAXTOK; i++)
	{
		token = strtok_r(NULL, " \t", &ptr);
		(*args)[i] = token;				// Save each argument token into args
		*argc = i;						// Keep track of # of arguments too
	}

	return 1;
}

/** 
 * @brief Executes commands from sh.c
 *
 * This function is called from within sh.c and is used to 
 * execute non-builtin commands with this shell. This function
 * determines what should happen if the shell does not recognize
 * the command. The first thing it does is check if the command
 * is a path that contains an executable. If not, then it performs
 * a which() type action to search the PATH for a matching path
 * that contains an executable. If this fails, this function
 * simply alerts the user that the command they entered was not found.
 *
 * @param command				The command to be executed
 * @param commandlineCONST		A constant copy of the commandline. Is not edited by this function, simply used for printing
 * @param args					The array of arguments passed into the shell, including the command
 * @param env					Environment variable array
 * @param pid					Parent process ID
 * @param pathlist				Pathelement struct that holds a linked list representation of PATH
 * @param status				Keeps track of the child process status if we call execute() and fork()
 * @param trigWild				Used to help format print output during execute()
 * @param bg			True if the process to be executed should be done in the background, false otherwise
 * @param redir			True if the parent has run a redirection command before calling this function, false otherwise
 */
void exec_command(char *command, char *commandlineCONST, char **args, char **env, pid_t pid, pathelement *pathlist, int status, bool trigWild, bool bg, bool redir)
{
	// Doesn't handle ./ or ../ ??
	if( (command[0] == '/') || ((command[0] == '.') && ((command[1] == '/') ||((command[1] == '.') && (command[2] == '/')))))
	{
		if (strstr(command, ".sh") == NULL) 
		{ 
			if (access(command, X_OK) == 0) 
			{ 
				execute(args[0], args, env, pid, status, trigWild, bg, redir); 
			} 
			else { perror("access denied"); }
		}	
		
		// This lets the shell attempt to execute .sh files if you give it one, however
		// .sh files only seem to run commands through the parent shell so this line
		// essentially just improves user experience theoretically
		else { execl("/bin/sh", "sh", "-c", command, (char *) 0); }	
	}
	
	else
	{
		// This does a which() and finds the first instance of the desired command in the PATH
		commandSet(pathlist, command, false, false);
		
		// Execute if we find a match
		if (command != NULL) 
		{ 
			if (access(command, X_OK) == 0) 
			{ 
				execute(command, args, env, pid, status, trigWild, bg, redir); 
			} 
			
			// Or print out command not found
			if (strstr(command, " Command not found") != NULL) 
			{
				if (!redir) { printf("%s: Command not found.\n", commandlineCONST); }
			}
		}		
	}
}