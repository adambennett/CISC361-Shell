#include "sh.h"

userList *usersHead = NULL; 								// Init head node for watchuser list
mailList *mailHead = NULL;									// Init head node for watchmail list

/** 
 * @brief Essentially the main() function of this program
 *
 * This is where the main processing during shell execution takes place. 
 * 			
 * @param argc   		Number of arguments passed into the shell when started
 * @param argv			String array of arguments passed into the shell program
 * @param envp			Environment variable
 *
 * @return Returns an int with a value indicating status of the return. 0 is returned in normal exit cases.
 */
int sh( int argc, char **argv, char **envp )
{
	// Global variables first defined in sh header
	memory = calloc(500, sizeof(char*));						// The saved command stack
	prompt = calloc(PROMPTMAX, sizeof(char));					// The prompt printed before [cwd]> before each user input
	bg = false;													// Set to true while the current command is to be backgrounded
	firstUser = true;											// Set to true until a user is added to the watchuser list
	mem = 0;													// Used in history to keep track of how many commands to print
	mems = 0;													// Used with history to keep track of how many commands have been saved
	status = 1;													// Stuff for fork()
	aliases = 0;												// Keeps track of # of aliases
	pathelement *pathlist = NULL;								// The struct that hold PATH info
	pid = getpid();												// Keeps track of the parent process ID
	pid_t return_pid;											// Keeps track of backgrounded child PIDs
	pthread_mutex_unlock(&watchuser_lock);						// Watchuser mutex lock can start unlocked
	
	// These are used to refer to the builtin commands at different parts of the program
	char *builtIns[1024] = {"cd", "which", "where", "pwd", "prev", "history", "pid", "kill", "list", "printenv", "setenv", "alias", "prompt", "hist", "refreshpath", "commands", "quit", "exit"};
	char *descrips[2048] = {"Changes directory.", "Finds the location of <arg> if it is executable", "Finds all the executable locations of <args>", "Prints the current directory", "Prints the previous directory",
							"Lists the last <arg> commands ran in mysh", "Prints the PID of the mysh process", "Sends a SIGTERM signal to <arg>, or sends <arg1> to <arg2>", 
							"Lists all files in <arg> or just the current dir.", "Prints the whole environment or just <arg> if that variable exists", "Sets an environment variable to <arg1>=<arg2>, or <arg1>=' '",
							"Allows the user to add aliases for other commands and then run those commands with the new aliases", "Allows the user to change the prompt the precedes the CWD to <arg1>",
							"History", "Refreshes the pathlist to the program's starting path.\n", "Lists this shell's built in commands and their functions", "Exits the shell", "Exits the shell" };
	
	// Init local variables for command processing
	char *commandline = calloc(MAX_CANON, sizeof(char));		// The dynamic copy of commandline that we manipulate as necessary (ex: remove trailing & for bg processes, remove redirection operators)
	char *commandlineCONST = calloc(MAX_CANON, sizeof(char));	// Keeps a constant copy of the full commandline passed by the user, no manipulation
	char **args = calloc(MAXARGS, sizeof(char*));				// The arguments passed into the shell. args[0] is the command, args[size] = NULL
	char **envMem = calloc(MAXMEM, sizeof(char*));				// Keeps a list of all the names of all env variables so that we can print them all using getenv()
	char **returnPtr = calloc(MAXMEM, sizeof(char*));			// Memory management helper
	char *mHelp = NULL;											// Memory management helper
	char *pathRtr = NULL;										// Memory management helper
	char *savedPath = getenv("PATH");							// Keeps a copy of the PATH env var during startup so later the user could theoretically reset it
	char command[2046];											// Just used right before executing a non-builtin to ensure the right string is passed into exec_command()
	bool go = true;												// Loop execution bool, keeps the program running until the user exits, terminates or causes problems
	bool clearedPath = false;									// Set to true whenever the user changes the PATH env variable until the pathlist is refreshed properly
	bool trigWild = false;										// Set to true during the loop if user passes in a wild arg. Used for print output cleanliness	
	bool external = false;										// Set to true if the shell executes an external command, reset before each fgets() loop
	bool redir = false;											// Set to true if a redirection operator is detected on the commandline
	bool ipc = false;											// Set to true if the user enters a piped command
	int argsc = 0;												// # of args passed to shell, including the command 
	int returns = 0;											// Used with the char** returnPtr to keep track of how many strings to free with plumber()
	int h = 0;													// Keeps track of the position in the char** memory to add the next saved command at
	int features = 17;											// # of built in functions, used in where/which to loop through builtins[]
	int fid;													// File descriptor used for properly handling file descriptors during redirection command processing
	int redirType = -2;											// Used to keep track of which type of redirection the users wishes to do
	
	// Setup prompt
	prompt[0] = ' '; prompt[1] = '\0';
	
	// Put PATH into a linked list
	char *mH = get_path(&pathlist);
	
	// Set pathlist head to the head of the list
	pathlist->head = pathlist;
	
	// Fill up the envMem array with all the names of the starting env variables for later reference during printenv
	fillEnvMem(envMem, envp);
	returnPtr[returns] = newEnvVar(envp, "CURDIR", getenv("PWD"), envMem); returns++;
	returnPtr[returns] = newEnvVar(envp, "PREVDIR", getenv("PWD"), envMem); returns++;
	returnPtr[returns] = newEnvVar(envp, "NOCLOB", "Not set", envMem); returns++;
	
	// Set every pathlist element's head to the same head 
	headRef(pathlist);
	
	// Loop until the user exits
	while (go)
	{
		// Handle Ctrl-C and Ctrl-Z (only during user input)
		signal(SIGINT, sigintHandler);
		signal(SIGTSTP, signalSTPHandler);
		signal(SIGCHLD, SIG_IGN);
		
		// Print the prompt (if we haven't already from the last loop)
		// Reset trigWild in case the user just passed a wild in the last input
		if (go) { fprintf(stderr, "%s[%s]>", prompt, getenv("CURDIR")); trigWild = false; }
		if (ipc) { ipc = false; }
		
		// Check user input
		while ((fgets(commandline, MAX_CANON, stdin) != NULL) && go) 
		{
			
			// If they somehow get NOTHING into commandline, just go again
			if (commandline == NULL) { printf("\n\n"); continue; }
			
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
			
			
			// Handle redirection
			redirType = proc_redirect(&commandline, fid);
			commandline = strtok(commandline, ">&<");
			if (redirType > 0) { redir = true; }
			else if (redirType == -3) { break; }
			
			// Handle IPC
			if (!redir)
			{
				//## Process IPC
				if(isIPC(commandlineCONST))
				{
					char *left, *right;
					int ipc_type = parse_ipc_line(&left, &right, commandlineCONST);
					perform_ipc(left, right, ipc_type, envp, envMem, pathlist);
					ipc = true;
				}
			}
			
			// Parse the line into a command and arguments
			int line = lineHandler(&argsc, &args, commandline);
			
			// If the line was parsed properly
			if (line == 1) 
			{
				// If the line is not just a [return]
				if (strcmp(commandlineCONST, "\n") != 0)
				{
					// Allocate a space on the saved command stack and save the commandline to it
					memory[h] = calloc(strlen(commandlineCONST) + 1, sizeof(char));
					strcpy(memory[h], commandlineCONST);
					
					// Move the position in the stack and set the counter for 'history'
					h++; mems++;
					if (mem < 10) { mem++; }
					if (mem > 10) { mem = 10; }
				}
			}
			
			// If IPC, we already executed everything we needed to so break and prompt the user for input again
			if (ipc) { break; }
			
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
				go = false;
				reset_redirection(&fid, redirType);
			}
		//	// WHICH : Prints out the first instance in 'PATH' of the input command
			else if ((strcmp(args[0], "which") == 0) || strcmp(args[0], "WHICH") == 0)
			{
				if (!redir) { printf("Executing built-in which\n"); }
				
				// Checks out the pathlist to make sure things are set correctly
				// User may have edited it with 'setenv'
				if (!clearedPath) { pathlist = pathlist->head; }
				if (mHelp != NULL) { free(mHelp); }
				mHelp = refreshPath(pathlist);
				headRef(pathlist);
				clearedPath = false;
				
				// Now find the first instance of command and print it
				which(args[1], builtIns, args[2], features, pathlist);
				
				reset_redirection(&fid, redirType);
			}
		//	// WHERE : Prints out every instance in 'PATH' of every input command
			else if ((strcmp(args[0], "where") == 0) || strcmp(args[0], "WHERE") == 0)
			{
				if (!redir) { printf("Executing built-in where\n"); }
				
				// Checks out the pathlist to make sure things are set correctly
				// User may have edited it with 'setenv'
				if (!clearedPath) { pathlist = pathlist->head; }
				if (mHelp != NULL) { free(mHelp); }
				mHelp = refreshPath(pathlist);
				headRef(pathlist);
				clearedPath = false;
				
				// Now find all instances of command and print them
				where(args, pathlist, builtIns, features);
				
				reset_redirection(&fid, redirType);
			}
		//	// CD : Changes directory	
			else if (strcmp(args[0], "cd") == 0)
			{
				if (!redir) { printf("Executing built-in cd\n"); }
				changeDirectory(envp,args, argsc, envMem);
				reset_redirection(&fid, redirType);
			}
		//	// PWD : Prints out the current directory
			else if ((strcmp(args[0], "pwd") == 0) || (strcmp(args[0], "PWD") == 0))
			{
				if (!redir) { printf("Executing built-in pwd\n%s\n", getenv("CURDIR")); }
				reset_redirection(&fid, redirType);
			}
		//	// PROMPT : Allows the user to alter the 'prompt' variable
			else if ((strcmp(args[0], "prompt") == 0) || (strcmp(args[0], "PROMPT") == 0))
			{
				if (!redir) { printf("Executing built-in prompt\n"); }
				prompter(args, prompt, argsc);
				reset_redirection(&fid, redirType);
			}
		//	// PID : Prints out the PID of the shell
			else if ((strcmp(args[0], "pid") == 0) || (strcmp(args[0], "PID") == 0))
			{
				if (!redir) { printf("Executing built-in pid\npid = %jd\n", (intmax_t) pid); }
				reset_redirection(&fid, redirType);
			}
		//	// HISTORY : Prints out the last X commands entered, X is 10 or the entered number
			else if ((strcmp(args[0], "history") == 0) || (strcmp(args[0], "HISTORY") == 0) || (strcmp(args[0], "hist") == 0))
			{
				if (!redir) { printf("Executing built-in history\n"); }
				hist(args, mem, memory, mems, argsc);
				reset_redirection(&fid, redirType);
			}
		//	// LIST : Prints out all the files in the current directory or the passed in directories
			else if ((strcmp(args[0], "list") == 0) || (strcmp(args[0], "LIST") == 0))
			{
				if (!redir) { printf("Executing built-in list\n"); }
				listHelper(argsc, getenv("CURDIR"), args);
				reset_redirection(&fid, redirType);
			}
		//	// PRINTENV : Prints out the environment	
			else if (strcmp(args[0], "printenv") == 0)
			{
				// This logic just makes sure that if we aren't going to print anything, we don't say we're executing printenv
				bool check = true;
				if (argsc > 1) { if (getenv(args[1]) == NULL) { check = false; } }
				if (check) 
				{ 
					if (!redir) { printf("Executing built-in printenv\n"); }
					envprint(envp, args, argsc, envMem);
				}
				reset_redirection(&fid, redirType);
			}
		//	// SETENV : Allows the user to alter the environment
			else if (strcmp(args[0], "setenv") == 0)
			{
				if (!redir) { printf("Executing built-in setenv\n"); }
				
				// Checks out the pathlist to make sure things are set correctly
				// User may have edited it with a previous 'setenv'
				if (!clearedPath) { pathlist = pathlist->head; }
				if (mHelp != NULL) { free(mHelp); }
				mHelp = refreshPath(pathlist);
				headRef(pathlist);
				clearedPath = false;
				
				// Set env variable according to user input and save a reference to something allocated during this process
				returnPtr[returns] = envSet(args, envp, &pathlist, argsc, envMem, pathRtr, clearedPath);
				returns++;
				reset_redirection(&fid, redirType);
			}
		//	// ALIAS : Prints out all the aliases or allows the user to create one	
			else if ((strcmp(args[0], "alias") == 0) || (strcmp(args[0], "ALIAS") == 0))
			{
				if (!redir) { printf("Executing built-in alias\n"); }
				aliases = proc_alias(aliasList, argsc, args, aliases);
				reset_redirection(&fid, redirType);
			}
		//	// KILL : Kills a passed in PID or sends the passed in signal to it
			else if ((strcmp(args[0], "kill") == 0) || (strcmp(args[0], "KILL") == 0) || (strcmp(args[0], "DESTROY") == 0))
			{
				if (!redir) { printf("Executing built-in kill\n"); }
				kill_proc(argsc, prompt, &memory, pathlist, commandlineCONST, &args, envMem, returnPtr, mHelp, mH, pathRtr, pid, aliases, aliasList, firstUser, tid1, mailHead, usersHead);
			}
			
		//	// WATCHMAIL : Allows users to track files and to be notified when a file changes in size
			else if ((strcmp(args[0], "watchmail") == 0) || (strcmp(args[0], "WATCHMAIL") == 0))
			{
				proc_watchmail(argsc, args);
				reset_redirection(&fid, redirType);
			}
		//	// WATCHUSER : Allows the user to track a user and to be notified when a tracked user logs in
			else if ((strcmp(args[0], "watchuser") == 0) || (strcmp(args[0], "WATCHUSER") == 0))
			{
				proc_watchuser(argsc, args, firstUser, tid1);
				reset_redirection(&fid, redirType);
			}
			
		//	// NOCLOBBER : Allows the user to modify the noclobber variable, the user cannot edit it via setenv
			else if ((strcmp(args[0], "noclobber") == 0) || ((strcmp(args[0], "NOCLOBBER") == 0)))
			{
				if (!redir) { printf("Executing built-in noclobber\n"); }
				if (strcmp(getenv("NOCLOB"), "Set") == 0) { setenv("NOCLOB", "Not set", 1); printf("Noclobber toggled off. Files will be overwritten.\n"); }
				else { setenv("NOCLOB", "Set", 1); printf("Noclobber toggled on. Files will NOT be overwritten.\n"); }
				reset_redirection(&fid, redirType);
			}
			
		//	// NEWLINE : Handles the case when the user just presses enter at the commandline
			else if (strcmp(args[0], "\n") == 0) { reset_redirection(&fid, redirType); }	

		//	// COMMANDS : Prints out all the built in commands of this shell
			else if (strcmp(args[0], "commands") == 0) 
			{
				for (int feat = 0; feat < features; feat++) { if (!redir) { printf("%s ---:--- %s\n", builtIns[feat], descrips[feat]); } }
				reset_redirection(&fid, redirType);
			}
			
		//	// REFRESH PATH : sets the pathlist back to where it started when the shell started up
			else if (strcmp(args[0], "refreshpath") == 0) 
			{
				setenv("PATH", savedPath, 1);
				free(mH);
				mH = get_path(&pathlist);
				pathlist->head = pathlist;
				headRef(pathlist);
				reset_redirection(&fid, redirType);
			}		

		//	// DEBUG : During development was used to print various things and to check values during debugging
			else if (strcmp(args[0], "debug") == 0) 
			{
				
			}
			
		//	// PREV : Prints out the previous directory
			else if ((strcmp(args[0], "prev") == 0) || (strcmp(args[0], "previous") == 0))
			{
				if (!redir) { printf("Executing built-in prev\n%s\n", getenv("PREVDIR")); }
				reset_redirection(&fid, redirType);
			}
			// END BUILT IN COMMANDS
			
			else 
			{
				// Toggle external
				external = true;
				
				// Checks out the pathlist to make sure things are set correctly
				// User may have edited it with 'setenv'
				if (!clearedPath) { pathlist = pathlist->head; }
				if (mHelp != NULL) { free(mHelp); }
				mHelp = refreshPath(pathlist);
				headRef(pathlist);
				clearedPath = false;
				
				// Ensure we're running the right thing here
				// Doesn't seem like this should be necessary, but doesn't work properly without it
				strcpy(command, args[0]);
				
				// Try to execute the command
				exec_command(command, commandlineCONST, args, envp, pid, pathlist, status, trigWild, bg, redir); 
				reset_redirection(&fid, redirType);
			}
			
			// Finish handling backgrounded commands
			if (bg && external)
			{
				// Do nonblocking waitpid() and keep track of the return
				return_pid = waitpid(-1, &status, WNOHANG);
				
				//Waitpid error
				if (return_pid == -1) { perror("Nonblocking waitpid"); } 
				
				// child is still running
				else if (return_pid == 0) {} 
				
				// child is finished
				else if (return_pid == pid) { signal(SIGCHLD, SIG_IGN); bg = false; }
			}
			
			
			// Ensure the prompt always shows up before harvesting user input
			// Prevents what looks like the shell is hanging when really it is just waiting for input
			if (go) 
			{ 
				fprintf(stderr, "%s[%s]>", prompt, getenv("CURDIR")); 
				
				// Make sure to also reset all necessary variables before re-looping
				argsc = 0;
				external = false;
				redir = false;
				redirType = -2;
				ipc = false;
			}
			// Here is where we exit the user input loop if they have quit the shell
			else { break; }
		}
	}
	
	// Free all allocated memory (ideally)
	plumber(prompt, &memory, pathlist, commandlineCONST, &args, envMem, returnPtr, mHelp, mH, pathRtr, true, aliases, aliasList, firstUser, tid1, mailHead, usersHead);
	
	// Close file descriptors (for valgrind)
	fclose( stdin );
	fclose( stdout );
	fclose( stderr );
	
	// EXIT
	return 0;
} 

/** 
 * @brief Used with which, where and for finding commands
 *
 * Loops through the path and creates a string of the absolute path
 * to the command.
 * 
 * @param pathlist			The path to search on
 * @param command   		The array of commands given to search for
 * @param cont				Set to true during which when we want to just find the first instance, causes the loop to break early
 * @param print          	Set to false when searching for a command to execute, because we don't want to print it in that case
 */
void commandSet(pathelement *pathlist, char *command, bool cont, bool print)
{
	bool found = false;
	char *str = malloc(256 * sizeof(char));
	pathelement *temp = pathlist->head;
	while (temp->next != NULL) 
	{
		strcpy(str, temp->element);
		strcat(str, "/");
		strcat(str, command);
		if (access(str, X_OK) == 0)
		{
			strcpy(command, str);
			found = true;
			if (print) { printf("%s\n", command); }
		}
		temp = temp->next;
		if (!cont && found) { break; }
		free(str);
		str = malloc(256 * sizeof(char));
	}
	if (found == false) 
	{ 
		strcat(command, ": Command not found."); 
		if (print) { printf("%s\n", command); }
	}
	free(str);
}

/** 
 * @brief Used with where
 *
 * Same as commandSet(), but it finds every instance of the command instead of just one
 * 
 * @param pathlist			The path to search on
 * @param command   		The array of commands given to search for
 * @param cont				Set to true during which when we want to just find the first instance, causes the loop to break early
 * @param print          	Set to false when searching for a command to execute, because we don't want to print it in that case
 */
void commandFind(pathelement *pathlist, char *command, bool cont, bool print)
{
	bool found = false;
	char *str = malloc(256 * sizeof(char));
	pathelement *temp = pathlist->head;
	while (temp->next != NULL) 
	{
		strcpy(str, temp->element);
		strcat(str, "/");
		strcat(str, command);
		if (access(str, X_OK) == 0)
		{
			found = true;
			if (print) { printf("%s\n", str); }
		}
		temp = temp->next;
	}
	if (found == false) 
	{ 
		char errStr[256];
		strcpy(errStr, command);
		strcat(errStr, ": Command not found."); 
		if (print) { printf("%s\n", errStr); }
	}
	free(str);
}

/** 
 * @brief Prints current PATH
 *
 * Loops through the pathlist (without destroying it)
 * and prints out each element.
 * 
 * @param pathlist			The path to search on
 */
void printPathlist(pathelement *pathlist)
{
	pathelement *temp = pathlist->head;
	while (temp->next != NULL) 
	{
		printf("Element: %s\n", temp->element);
		temp = temp->next;
	}
}

/** 
 * @brief Prints watchuser list
 *
 * Loops through the userlist (without destroying it)
 * and prints out each user.
 * 
 * @param usersHead			The list head to search from
 */
void printUsers(userList *usersHead)
{
	userList *temp = usersHead;
	if (temp->next == NULL && temp->node != NULL) 
	{
		printf("%s\n", temp->node);
	}
	while (temp->next != NULL) 
	{
		printf("%s\n", temp->node);
		temp = temp->next;
		if (temp->next == NULL) { printf("%s\n", temp->node); }
	}
}

/** 
 * @brief Prints watchmail list
 *
 * Loops through the maillist (without destroying it)
 * and prints out each file.
 * 
 * @param mailHead			The list head to search from
 */
void printMail(mailList *mailHead)
{
	mailList *temp = mailHead;
	if (temp->next == NULL && temp->filename != NULL) 
	{
		printf("%s\n", temp->filename);
	}
	while (temp->next != NULL) 
	{
		printf("%s\n", temp->filename);
		temp = temp->next;
		if (temp->next == NULL) { printf("%s\n", temp->filename); }
	}
}

/** 
 * @brief Counts watched users
 *
 * Loops through the userlist (without destroying it)
 * and counts each user.
 * 
 * @param usersHead			The list head to search from
 *
 * @return 0 if userlist is empty, otherwise returns the number of users being watched.
 */
int countUsers(userList *usersHead)
{
	if (usersHead == NULL) { return 0; }
	else 
	{
		int count = 0;
		userList *temp = usersHead;
		if (temp->next == NULL && temp->node != NULL) 
		{
			count++;
			return count;
		}
		while (temp->next != NULL) 
		{
			count++;
			temp = temp->next;
			if (temp->next == NULL) { count++; }
		}
		return count;
	}
}

/** 
 * @brief Counts watched files
 *
 * Loops through the maillist (without destroying it)
 * and counts each file.
 * 
 * @param mailHead			The list head to search from
 *
 * @return 0 if maillist is empty, otherwise returns the number of files being watched.
 */
int countMail(mailList *mailHead)
{
	if (mailHead == NULL) { return 0; }
	else 
	{
		int count = 0;
		mailList *temp = mailHead;
		if (temp->next == NULL && temp->filename != NULL) 
		{
			count++;
			return count;
		}
		while (temp->next != NULL) 
		{
			count++;
			temp = temp->next;
			if (temp->next == NULL) { count++; }
		}
		return count;
	}
}

/** 
 * @brief Checks userslist for username
 *
 * Loops through the userlist (without destroying it)
 * and checks to see if the given username is already being watched.
 * 
 * @param usersHead			The list head to search from
 * @param userName			The name to search for
 *
 * @return True if the username is found in the list, or false otherwise.
 */
bool isUser(userList *usersHead, char *userName)
{
	if (usersHead == NULL) { return false; }
	else 
	{
		userList *temp = usersHead;
		if (temp->next == NULL && temp->node != NULL) 
		{
			if (strcmp(userName, temp->node) == 0) { return true; }
		}
		while (temp->next != NULL) 
		{
			if (strcmp(userName, temp->node) == 0) { return true; }
			temp = temp->next;
			if (temp->next == NULL) 
			{
				if (strcmp(userName, temp->node) == 0) { return true; }
			}
		}
		return false;
	}
}

/** 
 * @brief Checks maillist for filename
 *
 * Loops through the maillist (without destroying it)
 * and checks to see if the given file is already being watched.
 * 
 * @param mailHead			The list head to search from
 * @param fileName			The filename to search for
 *
 * @return True if the file is found in the list, or false otherwise.
 */
bool isMail(mailList *mailHead, char *fileName)
{
	if (mailHead == NULL) { return false; }
	else 
	{
		mailList *temp = mailHead;
		if (temp->next == NULL && temp->filename != NULL) 
		{
			if (strcmp(fileName, temp->filename) == 0) { return true; }
		}
		while (temp->next != NULL) 
		{
			if (strcmp(fileName, temp->filename) == 0) { return true; }
			temp = temp->next;
			if (temp->next == NULL) 
			{
				if (strcmp(fileName, temp->filename) == 0) { return true; }
			}
		}
		return false;
	}
}

/** 
 * @brief Checks if a directory is open-able
 *
 * Used with list to help print nice output, in case the user passes in
 * any directories that do not exist or cannot be opened for whatever reason.
 * 
 * @param dir				Directory to check
 *
 * @return Returns 0 if the directory is not accessible, or 1 if it is.
 */
int listCheck(char *dir)
{
	DIR* directory;
	directory = opendir(dir);
	if(directory == NULL) { return 0; }
	else { closedir(directory); return 1; } 
}

/** 
 * @brief Used with list to print
 *
 * Calls the list() command and uses the results to print out the desired output
 * depending on what arguments the user has passed to the shell
 * 
 * @param argsc				Number of arguments passed to the commandline (including list)
 * @param owd   			The current directory in case the user only enters 'list'
 * @param args				The arguments passed into the commandline
 */
void listHelper(int argsc, char *owd, char **args)
{
	if (argsc == 1) { printf("%s: \n", owd); list(owd); }
	else
	{
		int i;
		for(i = 1; args[i] != NULL; i++)
		{
			if (listCheck(args[i]) > 0)
			{
				printf("\n");
				printf("%s: \n", args[i]);
				list(args[i]);
			}
			else
			{
				printf("\n");
				printf("Cannot open directory: %s\n", args[i]);
			}
		}
	}
} 
	
/** 
 * @brief Check if last character in given string is '&'
 *
 * Checks the passed in string to see if the last character is & or not.
 * 
 * @param str				The string to check
 *
 * @return Returns 0 if the last character is &, or 1 otherwise
 */	
int lastChar(const char *str)
{
	if(*str && str[strlen(str + 1)] == '&') { return 0; }
	else { return 1; }
}

/** 
 * @brief Called by the watchuser thread
 *
 * This is the routine run by the thread that is started when a user
 * enters the watchuser command. The thread runs this function,
 * which, every 20 seconds, prints out any watched users who are currently logged in.
 * 
 * @param param				The name of the thread
 */
void *watchuser(void *param)
{
	struct utmpx *up;		// Holds information about who is currently using the system
	
	// Continuously run this loop while the thread is still active
	while(1)
	{
		// Wait 20 seconds between each alert
		sleep(20);
		
		// Reset database
		setutxent();
		
		// While the database has more entries, keep looking through them
		while((up = getutxent() ))
		{
			// Only care about users
			if (up->ut_type == USER_PROCESS)
			{
				// Lock so other threads cannot potentially edit global variables used below during execution
				pthread_mutex_lock(&watchuser_lock);
				
				// Create copy of usersHead so as not to destroy the list while traversing
				userList *temp = usersHead;
				
				// Look through every entry in userlist
				while (temp != NULL)
				{
					// If the entry matches a user from the database, they are logged in and the user is watching them
					if (strcmp(temp->node, up->ut_user) == 0)
					{
						// So print out the alert, then reprompt the user for input as this alert likely screwed up the pretty formatting
						printf("%s had logged on %s from %s\n", up->ut_user, up->ut_line, up->ut_host);
						fprintf(stderr, "%s[%s]>", prompt, getenv("CURDIR")); 
					}
					temp = temp->next;
				}
				
				// Unlock now after above execution is complete
				pthread_mutex_unlock(&watchuser_lock);
			}
		}
	}
}

/** 
 * @brief Called by the watchmail thread(s)
 *
 * This is the routine run by the thread(s) that are started when a user
 * enters the watchmail command. The thread(s) run this function,
 * which alerts the user if the associated file's size is changed.
 * This routine is run by each thread in the watchmail list, which
 * allows alerts to show up simultaneously if two or more files are
 * changed at the same time.
 * 
 * @param param				The name of the thread
 */
void *watchmail(void *param)
{
	
	const char *file_name = param;	//set up file name
	struct tm* ptm;					//used for formatting time_string
	struct stat file_Stat;			//define struct so we can find the size of the file
	struct timeval TIME;			//define struct so we can get the time
	char time_string[40];			//holds and prints the time of modification
	int prev_file_Size = -1;		//keeps track of old file size for comparison

	// Continue to run this loop while the thread is active, because we want to keep alerting more than once
	while(1)
	{
		// Create temp list so as to not destroy the mailHead list
		mailList *temp = mailHead;
		int found = 0;
		
		// Loop through mailHead looking for the file associated with the thread running this routine currently
		// Check first entry of mailList to see if it matches the file
		if (temp->next == NULL) { if(strcmp(temp->filename,file_name) == 0) { found = 1;}}
		
		// Check other entries
		while(temp->next != NULL)
		{
			if(strcmp(temp->filename,file_name) == 0) { found = 1; break; }
			else
			{
				// If no match yet, loop through list by moving to next node here
				temp = temp->next;
				
				// Ensure last entry is checked
				if (temp->next == NULL) { if(strcmp(temp->filename,file_name) == 0) { found = 1; } }
			}
		}
		
		// If the file is in the list, that means the user is still watching it and the thread is still running
		if(found == 1)
		{
			// Get current file size
			stat(file_name, &file_Stat);
			
			// If first time running this routine, just setup prev_file_size to be the size of the file
			if (prev_file_Size == -1) { prev_file_Size = file_Stat.st_size; }
			
			// Check if file size if larger than the previous size
			if (file_Stat.st_size > prev_file_Size)
			{
				// If so, check the time
				gettimeofday(&TIME, NULL);
				ptm = localtime (&TIME.tv_sec);
				strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
				
				// Print file change prompt
				printf("BEEP\a You've Got Mail in %s at %s\n", file_name, time_string);
				
				// Edit the previous size for the next loop
				prev_file_Size = file_Stat.st_size;
			}
			sleep(1);
		}
	}
}

/** 
 * @brief Adds a user to watched users list
 *
 * Takes the name of a user and adds it to the list
 * of users being watched for the watchuser command.
 * 
 * @param userName				Name of user to add to list
 * @param usersHead				Pointer to list to add user to
 */
void addUser(char *userName, userList **usersHead)
{
	// If this user is already being watched, just skip this
	if (isUser(*usersHead, userName)) {}
	
	// Otherwise
	else 
	{
		// Setup node for user
		userList *new_node = malloc(sizeof(userList));
		new_node->node = malloc(1024);
		strcpy(new_node->node, userName);
		new_node->next = NULL;
		
		// If this is the first new user, usersHead IS the user now
		if(!*usersHead) { *usersHead = new_node; }
		
		// Otherwise there are already users being watched, so add new node to the end of the list
		else
		{
			// Temp copy of usersHead for non-destructive traversal
			userList *temp = *usersHead;
			
			// Check all entries to see where end of list is
			// Add new node to the end of the list
			if (temp->next == NULL) { temp->next = new_node; *usersHead = temp; }
			else 
			{
				while (temp->next != NULL) { temp = temp->next; }
				temp->next = new_node; 
			}
		}
	}
}

/** 
 * @brief Adds a file to watched files list
 *
 * Takes the name of a file and adds it to the list
 * of files being watched for the watchmail command.
 * 
 * @param fileName				Name of file to add to list
 * @param mailHead				Pointer to list to add file to
 */
void addMail(char *fileName, mailList **mailHead)
{
	// If the file is already being watched, just skip this
	if (isMail(*mailHead, fileName)) { }
	
	// Otherwise
	else 
	{
		// Create thread reference
		pthread_t tid;
		
		// Setup node for file
		mailList *new_node = malloc(sizeof(mailList));
		new_node->filename = malloc(1024);
		strcpy(new_node->filename, fileName);
		new_node->next = NULL;
		
		// Associate thread reference with node
		new_node->thread = tid;
		
		// If this is the first new file, mailHead IS the file now
		if(!*mailHead) { *mailHead = new_node; }
	
		// Otherwise there are already files being watched, so add new node to the end of the list
		else
		{
			// Temp copy of mailHead for non-destructive traversal
			mailList *temp = *mailHead;
			
			// Check all entries to see where end of list is
			// Add new node to the end of the list
			if (temp->next == NULL) { temp->next = new_node; *mailHead = temp; }
			else 
			{
				while (temp->next != NULL) { temp = temp->next;}
				temp->next = new_node; 
			}
		}

		// Now start thread from the node
		if (pthread_create(&(new_node->thread), NULL, watchmail, (void*)(new_node->filename)) != 0) { perror("mail thread start"); } 
	}
}

/** 
 * @brief Removes user from watched list
 *
 * Removes given username from the list of watched users.
 * 
 * @param userName				Name of user to remove
 * @param head					Pointer to list to remove from
 *
 * @return Returns true if a user was removed, or false if not.
 */
bool removeUser(char *userName, userList **head) 
{
    if (strcmp(userName, ((*head)->node)) == 0) 
	{
        userList *temp = *head;
        *head = (*head)->next;
        free(temp);
        return true;
    }
    userList *current = (*head)->next;
    userList *previous = *head;
    while (current != NULL && previous != NULL) 
	{
        if (strcmp(userName, current->node) == 0) 
		{
			userList *temp = current;
            previous->next = current->next;
            free(temp);
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

/** 
 * @brief Removes file from watched list
 *
 * Removes given filename from the list of watched files.
 * 
 * @param fileName				Name of file to remove
 * @param head					Pointer to list to remove from
 *
 * @return Returns true if a file was removed, or false if not.
 */
bool removeMail(char *fileName, mailList **head) 
{
    if (strcmp(fileName, ((*head)->filename)) == 0) 
	{
        mailList *temp = *head;
        *head = (*head)->next;
		if (pthread_cancel(temp->thread) != 0) { int ex = -1; pthread_exit(&ex); }
        free(temp);
        return true;
    }
    mailList *current = (*head)->next;
    mailList *previous = *head;
    while (current != NULL && previous != NULL) 
	{
        if (strcmp(fileName, current->filename) == 0) 
		{
			mailList *temp = current;
            previous->next = current->next;
			if (pthread_cancel(temp->thread) != 0) { int ex = -1; pthread_exit(&ex); }
            free(temp);
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

/** 
 * @brief Called when user enters watchuser
 *
 * Handles all the necessary logic when a user enters
 * the watchuser command into the shell. This function
 * will print out the correct output for the command
 * based on the number of args the user passes as well as
 * what the arguments are. 
 * 
 * @param argsc				Number of arguments, including the command
 * @param args				Array of arguments passed with the watchuser command
 * @param firstUser			Set to true if there is not a watchuser thread spinning already
 * @param tid1				Reference to watchuser thread that is created the first time we watch a new user
 */
void proc_watchuser(int argsc, char **args, bool firstUser, pthread_t tid1)
{
	// User entered "watchuser"
	if (argsc == 1)
	{
		// Make sure there are users being watched
		if (countUsers(usersHead) > 0)
		{
			// Print watched users
			printf("Executing built-in watchuser\n");
			printf("Watched Users\n");
			printUsers(usersHead);
		}
	}
	
	// User entered "watchuser <username>"
	else if (argsc == 2)
	{
		printf("Executing built-in watchuser\n");
		
		// If this is the first watched user since starting the shell
		if (firstUser)
		{
			// Spin up a thread to watch all users
			if (pthread_create(&tid1, NULL, watchuser, "Watchuser Thread") != 0) { perror("user thread start"); }
			firstUser = false;
		}
		
		// Lock mutex so not other processes may edit global linked list
		pthread_mutex_lock(&watchuser_lock);
		
		// Add user to list of watched users
		addUser(args[1], &usersHead);
		
		// Unlock the mutex after we're done with the global linked list
		pthread_mutex_unlock(&watchuser_lock);
	}
	
	// User entered "watchuser <username> off"
	else if (argsc == 3)
	{
		printf("Executing built-in watchuser\n");
		
		// Make sure args[2] is "off"
		if (strcmp(args[2], "off") == 0)
		{
			// Lock the mutex before editing global linked list
			pthread_mutex_lock(&watchuser_lock);
			
			// If the user is removed from the list
			if (removeUser(args[1], &usersHead)) { printf("No longer watching %s\n", args[1]); }
			
			// Otherwise, that user was not being watched
			else { printf("No entries found\n"); }
			
			// Unlock the mutex after we are done with the global list
			pthread_mutex_unlock(&watchuser_lock);
		}
		
		// User entered "watchuser <username> <garbage>"
		else { printf("Improper usage of watchuser.\n"); }
	}
	
	// User entered "watchuser ..." with more than 3 total args
	else if (argsc > 3)
	{
		printf("Executing built-in watchuser\n");
		printf("watchuser: too many arguments\n");
	}
}

/** 
 * @brief Called when user enters watchmail
 *
 * Handles all the necessary logic when a user enters
 * the watchmail command into the shell. This function
 * will print out the correct output for the command
 * based on the number of args the user passes as well as
 * what the arguments are. 
 * 
 * @param argsc				Number of arguments, including the command
 * @param args				Array of arguments passed with the watchuser command
 */
void proc_watchmail(int argsc, char **args)
{
	
	// User entered "watchmail"
	if (argsc == 1)
	{
		// Make sure there are watched files to print
		if (countMail(mailHead) > 0)
		{
			// Print watched files
			printf("Executing built-in watchmail\n");
			printf("Watched Mail\n");
			printMail(mailHead);
		}
	}
	
	// User entered "watchmail <filename>"
	else if(argsc == 2)
	{
		printf("Executing built-in watchmail\n");
	
		// If file exists, add to watched mail list
		if (access(args[1], F_OK) != -1) { addMail(args[1], &mailHead); }
		
		// Otherwise print file not found alert
		else { printf("Cannot find the file: %s\n", args[1]); }
	}
	
	// User entered "watchmail <filename> off"
	else if (argsc == 3)
	{
		printf("Executing built-in watchmail\n");
	
		// Make sure args[2] is "off"
		if (strcmp(args[2], "off") == 0)
		{
			// If the file can be removed from the mail list
			if (removeMail(args[1], &mailHead)) { printf("No longer watching %s\n", args[1]); }
			
			// Otherwise, print file does not exist in mail list alert
			else { printf("No entries found\n"); }
		}
		
		// User entered "watchmail <filename> <garbage>"
		else { printf("Improper usage of watchmail.\n"); }
	}
	
	// User entered "watchmail ... " with more than 3 total arguments
	else if (argsc > 3)
	{
		printf("Executing built-in watchmail\n");
		printf("watchmail: too many arguments\n");
	}
}