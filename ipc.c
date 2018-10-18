#include "sh.h"

const char* IPC_OPERATORS[] = { "|&", "|" };    
const int NUM_IPC_OPERATORS = 5;

/** 
 * @brief Determines if line contains IPC operator
 *
 * @param line   			The string to check
 *
 * @return Returns true if the line contains '|' or '|&', or false otherwise.
 */
bool isIPC(char* line)
{
    return strstr(line, "|") || strstr(line, "|&");
}

/** 
 * @brief Determines IPC operator type and left/right commands
 *
 * NOTE: Line should be checked for IPC operator before calling this
 * 			
 * @param left   			The left command
 * @param right				The right command
 * @param line				The commandline passed in
 *
 * @return Returns 0 if the operator is '|&', or 1 if it is '|'.
 */
int parse_ipc_line(char** left, char** right, char* line){

    char* p = NULL;
	int ipc_code;
    for(int i=0; i < NUM_IPC_OPERATORS && p == NULL; i++)
	{
        p = strstr(line, IPC_OPERATORS[i]);
        ipc_code = i;
    }
	
	//This logic sucks, luckily it works though
    int left_length = (int)p - (int)line;
    *left = (char*)malloc(left_length + 1);
    memcpy(*left, line, left_length + 1);
    (*left)[left_length - 1] = '\0';

    char* ptr = strtok(line + left_length, "|&");
    int right_length = strlen(line) - (int)p + (int)line;
    *right = (char*)malloc(right_length + 1);
    memcpy(*right, ptr, right_length + 1);
	
    return ipc_code;
}

/** 
 * @brief Performs the actual IPC operation
 *
 * Runs the left and right commands, opens/closes/dupes file descriptors
 * 			
 * @param left   			The left command
 * @param right				The right command
 * @param ipc_type			The IPC operator code from parse_ipc_line()
 * @param envp				The environment
 * @param envMem			Used during proc_command to handle printenv and setenv properly
 * @param pathlist			Pointer to pathlist for proc_command
 */
void perform_ipc(char* left, char* right, int ipc_type, char **envp, char **envMem, pathelement *pathlist)
{
    int fid;
    int filedes[2];     	// Index 0 will be a dup of stdin and index 1 a dup of stdout/stderr
                        
    if(pipe(filedes) == -1)
	{
        perror("Error creating pipe");
        return;
    }

    // Redirect stdin
    close(0);
    dup(filedes[0]);
    close(filedes[0]);

    // Redirect stdout
    close(1);
    dup(filedes[1]);

    // Redirect stderr
    if(ipc_type == 0)
	{
        close(2);
        dup(filedes[1]);
    }

    close(filedes[1]);

    // Run the command on the left
	proc_command(left, envp, envMem, pathlist);

    // Return stdout to terminal
    fid = open("/dev/tty", O_WRONLY);
    close(1);
    dup(fid);
    close(fid);

    // Return sterr to terminal
    fid = open("/dev/tty", O_WRONLY);
    close(2);
    dup(fid);
    close(fid);

    // Run the command on the right
	proc_command(right, envp, envMem, pathlist);

    // Return stdin to terminal
    fid = open("/dev/tty", O_RDONLY);
    close(0);
    dup(fid);
    close(fid);
}

/** 
 * @brief Runs builtin and external commands
 *
 * Does a cheap version of what happens during the user input loop, used during IPC handling.
 * 			
 * @param commandline		The commandline we found
 * @param envp				The environment
 * @param envMem			Used to handle printenv and setenv properly
 * @param pathlist			Pointer to pathlist
 *
 */
void proc_command(char *commandline, char **envp, char **envMem, pathelement *pathlist)
{
	char *commandlineCONST = malloc(strlen(commandline) + 1);
	char **args = calloc(MAXARGS, sizeof(char*));
	char *builtIns[1024] = {"cd", "which", "where", "pwd", "prev", "history", "pid", "kill", "list", "printenv", "setenv", "alias", "prompt", "hist", "refreshpath", "commands", "quit", "exit"};
	char *descrips[2048] = {"Changes directory.", "Finds the location of <arg> if it is executable", "Finds all the executable locations of <args>", "Prints the current directory", "Prints the previous directory",
							"Lists the last <arg> commands ran in mysh", "Prints the PID of the mysh process", "Sends a SIGTERM signal to <arg>, or sends <arg1> to <arg2>", 
							"Lists all files in <arg> or just the current dir.", "Prints the whole environment or just <arg> if that variable exists", "Sets an environment variable to <arg1>=<arg2>, or <arg1>=' '",
							"Allows the user to add aliases for other commands and then run those commands with the new aliases", "Allows the user to change the prompt the precedes the CWD to <arg1>",
							"History", "Refreshes the pathlist to the program's starting path.\n", "Lists this shell's built in commands and their functions", "Exits the shell", "Exits the shell" };
	bool trigWild = false;
	int features = 17;	
	int argsc = 0;
	
	// Otherwise it almost always should have a newline last, but check anyway
	if (commandline[strlen(commandline) - 1] == '\n')
	{
		// replace newline with null
		if (strlen(commandline) > 1) { commandline[strlen(commandline) - 1] = 0; } 
	}
	
	// Save a copy of this formatted commandline
	strcpy(commandlineCONST, commandline);
	
	if (lastChar(commandline) == 0) 
	{
		bg = true;
		if (strlen(commandline) > 1) { commandline[strlen(commandline) - 1] = 0; } 
	}
	else { bg = false; }
	
	
	// Parse the line into a command and arguments
	lineHandler(&argsc, &args, commandline);
	
	// Handle wildcards
	if (hasWildcards(commandlineCONST)) 
	{ 
		// Transform args into the proper commandline after handling wildcards
		args = expand(args, argsc);
		trigWild = true;
	}
	
	// Check if command is an alias before processing it further
	if (isAlias(args[0], aliasList, 1, aliases))
	{
		// This function changes args[0] to be the proper command if the user did pass an alias
		if (morphAlias(args[0], aliasList, aliases) == 0)
		{
			perror("alias");
		}
	}
	
	// BUILT IN COMMANDS //
//	// EXIT : Exits the shell
	if ((strcmp(args[0], "exit") == 0) || (strcmp(args[0], "EXIT") == 0) || (strcmp(args[0], "quit") == 0))
	{
		printf("Exiting shell..\n");
		//go = false;
		//reset_redirection(&fid, redirType);
	}
//	// WHICH : Prints out the first instance in 'PATH' of the input command
	else if ((strcmp(args[0], "which") == 0) || strcmp(args[0], "WHICH") == 0)
	{
		printf("Executing built-in which\n");
		// Checks out the pathlist to make sure things are set correctly
		// User may have edited it with 'setenv'
		//if (!clearedPath) { pathlist = pathlist->head; }
		//if (mHelp != NULL) { free(mHelp); }
		//mHelp = refreshPath(pathlist);
		//headRef(pathlist);
		//clearedPath = false;
		
		// Now find the first instance of command and print it
		which(args[1], builtIns, args[2], features, pathlist);
		
		//reset_redirection(&fid, redirType);
	}
//	// WHERE : Prints out every instance in 'PATH' of every input command
	else if ((strcmp(args[0], "where") == 0) || strcmp(args[0], "WHERE") == 0)
	{
		printf("Executing built-in where\n");
		// Checks out the pathlist to make sure things are set correctly
		// User may have edited it with 'setenv'
		//if (!clearedPath) { pathlist = pathlist->head; }
		//if (mHelp != NULL) { free(mHelp); }
		//mHelp = refreshPath(pathlist);
		//headRef(pathlist);
		//clearedPath = false;
		
		// Now find all instances of command and print them
		where(args, pathlist, builtIns, features);
		
		//reset_redirection(&fid, redirType);
	}
//	// CD : Changes directory	
	else if (strcmp(args[0], "cd") == 0)
	{
		printf("Executing built-in cd\n");
		changeDirectory(envp,args, argsc, envMem);
		//reset_redirection(&fid, redirType);
	}
//	// PWD : Prints out the current directory
	else if ((strcmp(args[0], "pwd") == 0) || (strcmp(args[0], "PWD") == 0))
	{
		printf("Executing built-in pwd\n%s\n", getenv("CURDIR"));
		//reset_redirection(&fid, redirType);
	}
//	// PROMPT : Allows the user to alter the 'prompt' variable
	else if ((strcmp(args[0], "prompt") == 0) || (strcmp(args[0], "PROMPT") == 0))
	{
		printf("Executing built-in prompt\n");
		prompter(args, prompt, argsc);
		//reset_redirection(&fid, redirType);
	}
//	// PID : Prints out the PID of the shell
	else if ((strcmp(args[0], "pid") == 0) || (strcmp(args[0], "PID") == 0))
	{
		printf("Executing built-in pid\npid = %jd\n", (intmax_t) pid);
		//reset_redirection(&fid, redirType);
	}
//	// HISTORY : Prints out the last X commands entered, X is 10 or the entered number
	else if ((strcmp(args[0], "history") == 0) || (strcmp(args[0], "HISTORY") == 0) || (strcmp(args[0], "hist") == 0))
	{
		printf("Executing built-in history\n");
		hist(args, mem, memory, mems, argsc);
		//reset_redirection(&fid, redirType);
	}
//	// LIST : Prints out all the files in the current directory or the passed in directories
	else if ((strcmp(args[0], "list") == 0) || (strcmp(args[0], "LIST") == 0))
	{
		printf("Executing built-in list\n");
		listHelper(argsc, getenv("CURDIR"), args);
		//reset_redirection(&fid, redirType);
	}
//	// PRINTENV : Prints out the environment	
	else if (strcmp(args[0], "printenv") == 0)
	{
		// This logic just makes sure that if we aren't going to print anything, we don't say we're executing printenv
		bool check = true;
		if (argsc > 1) { if (getenv(args[1]) == NULL) { check = false; } }
		if (check) 
		{ 
			printf("Executing built-in printenv\n");
			envprint(envp, args, argsc, envMem);
		}
		//reset_redirection(&fid, redirType);
	}
//	// SETENV : Allows the user to alter the environment
	else if (strcmp(args[0], "setenv") == 0)
	{
		printf("Executing built-in setenv\n");
		
		// Checks out the pathlist to make sure things are set correctly
		// User may have edited it with a previous 'setenv'
		//if (!clearedPath) { pathlist = pathlist->head; }
		//if (mHelp != NULL) { free(mHelp); }
		//mHelp = refreshPath(pathlist);
		//headRef(pathlist);
		//clearedPath = false;
		char *temp = NULL;
		// Set env variable according to user input and save a reference to something allocated during this process
		envSet(args, envp, &pathlist, argsc, envMem, temp, true);
		//reset_redirection(&fid, redirType);
	}
//	// ALIAS : Prints out all the aliases or allows the user to create one	
	else if ((strcmp(args[0], "alias") == 0) || (strcmp(args[0], "ALIAS") == 0))
	{
		printf("Executing built-in alias\n");
		aliases = proc_alias(aliasList, argsc, args, aliases);
		//reset_redirection(&fid, redirType);
	}
//	// KILL : Kills a passed in PID or sends the passed in signal to it
	else if ((strcmp(args[0], "kill") == 0) || (strcmp(args[0], "KILL") == 0) || (strcmp(args[0], "DESTROY") == 0))
	{
		//printf("Executing built-in kill\n");
		//kill_proc(argsc, prompt, &memory, pathlist, commandlineCONST, &args, envMem, returnPtr, mHelp, mH, pathRtr, pid, aliases, aliasList, firstUser, tid1, mailHead);
		// call reset_redirection inside kill_proc
	}
	
//	// WATCHMAIL : Allows users to track files and to be notified when a file changes in size
	else if ((strcmp(args[0], "watchmail") == 0) || (strcmp(args[0], "WATCHMAIL") == 0))
	{
		proc_watchmail(argsc, args);
		//reset_redirection(&fid, redirType);
	}
//	// WATCHUSER : Allows the user to track a user and to be notified when a tracked user logs in
	else if ((strcmp(args[0], "watchuser") == 0) || (strcmp(args[0], "WATCHUSER") == 0))
	{
		proc_watchuser(argsc, args, firstUser, tid1);
		//reset_redirection(&fid, redirType);
	}
	
//	// NOCLOBBER : Allows the user to modify the noclobber variable, the user cannot edit it via setenv
	else if ((strcmp(args[0], "noclobber") == 0) || ((strcmp(args[0], "NOCLOBBER") == 0)))
	{
		printf("Executing built-in noclobber\n");
		if (strcmp(getenv("NOCLOB"), "Set") == 0) { setenv("NOCLOB", "Not set", 1); printf("Noclobber toggled off. Files will be overwritten.\n"); }
		else { setenv("NOCLOB", "Set", 1); printf("Noclobber toggled on. Files will NOT be overwritten.\n"); }
		//reset_redirection(&fid, redirType);
	}
	
//	// NEWLINE : Handles the case when the user just presses enter at the commandline
	else if (strcmp(args[0], "\n") == 0) { }	

//	// COMMANDS : Prints out all the built in commands of this shell
	else if (strcmp(args[0], "commands") == 0) 
	{
		for (int feat = 0; feat < features; feat++) { printf("%s ---:--- %s\n", builtIns[feat], descrips[feat]); }
		//reset_redirection(&fid, redirType);
	}

	
//	// PREV : Prints out the previous directory
	else if ((strcmp(args[0], "prev") == 0) || (strcmp(args[0], "previous") == 0))
	{
		printf("Executing built-in prev\n%s\n", getenv("PREVDIR"));
	}
	// END BUILT IN COMMANDS
	
	else 
	{
		// Toggle external
		//external = true;
		
		// Checks out the pathlist to make sure things are set correctly
		// User may have edited it with 'setenv'
		//if (!clearedPath) { pathlist = pathlist->head; }
		////if (mHelp != NULL) { free(mHelp); }
		///mHelp = refreshPath(pathlist);
		//headRef(pathlist);
		//clearedPath = false;
		
		// Ensure we're running the right thing here
		// Doesn't seem like this should be necessary, but doesn't work without it
		//strcpy(command, args[0]);
		
		// Try to execute the command
		exec_command(args[0], commandlineCONST, args, envp, pid, pathlist, status, trigWild, bg, false); 
		//reset_redirection(&fid, redirType);
	}
}