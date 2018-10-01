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
 *
 * @return Returns the status of the child process after waitpid() completes in the parent.
 */
int execute(char *cmd, char **argv, char **env, pid_t pid, int status, bool trigWild)
{

	pid = fork();
    int child_status = status;

    if(pid == 0) //** Executed in child process
	{                        
		char temp[2046];
		strcpy(temp, argv[0]);
		if (!trigWild) { for (int i = 1; argv[i] != NULL; i++) { strcat(temp, " "); strcat(temp, argv[i]); } }
		printf("Executing %s\n", temp);
		//printf("That came from execute() inside execute.c\n\n");
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
 * @param argv			Array to be filled with only the arguments after the initial command from the commandline
 * @param commandline	String that contains the users input from the sh.c fgets() loop
 *
 * @return Returns 1 if the line is parsed properly. Returns 0 if you try to parse a blank line.
 */
int lineHandler(int *argc, char ***args, char ***argv, char *commandline)
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
		(*args)[i] = token;				// args puts them right at element 'i' since we wanted command in 0
		(*argv)[i-1] = token;			// argv wants the arguments starting at element 0 though, so do i - 1
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
 * @param argsEx				The array of arguments passed into the shell, including the command
 * @param env					Environment variable array
 * @param pid					Parent process ID
 * @param pathlist				Pathelement struct that holds a linked list representation of PATH
 * @param status				Keeps track of the child process status if we call execute() and fork()
 * @param trigWild				Used to help format print output during execute() 
 */
void exec_command(char *command, char *commandlineCONST, char **argsEx, char **env, pid_t pid, pathelement *pathlist, int status, bool trigWild)
{
	// Doesn't handle ./ or ../ ??
	if( (command[0] == '/') || ((command[0] == '.') && ((command[1] == '/') ||((command[1] == '.') && (command[2] == '/')))))
	{
		if (strstr(command, ".sh") == NULL) 
		{ 
			if (access(command, X_OK) == 0) { execute(argsEx[0], argsEx, env, pid, status, trigWild); } 
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
		
		// Then we either execute if we find a match, or print out command not found
		if (command != NULL) { execute(command, argsEx, env, pid, status, trigWild); }
		else { printf("%s: Command not found.\n", commandlineCONST); }
	}
}