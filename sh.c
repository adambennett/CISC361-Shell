#include "sh.h"

userList *usersHead = NULL; 								// Init head node for watchuser list
userList *usersTail = NULL; 								// Init tail node for watchuser list

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
	// These are used to refer to the builtin commands at different parts of the program
	char *builtIns[1024] = {"cd", "which", "where", "pwd", "prev", "history", "pid", "kill", "list", "printenv", "setenv", "alias", "prompt", "hist", "refreshpath", "commands", "quit", "exit"};
	char *descrips[2048] = {"Changes directory.", "Finds the location of <arg> if it is executable", "Finds all the executable locations of <args>", "Prints the current directory", "Prints the previous directory",
							"Lists the last <arg> commands ran in mysh", "Prints the PID of the mysh process", "Sends a SIGTERM signal to <arg>, or sends <arg1> to <arg2>", 
							"Lists all files in <arg> or just the current dir.", "Prints the whole environment or just <arg> if that variable exists", "Sets an environment variable to <arg1>=<arg2>, or <arg1>=' '",
							"Allows the user to add aliases for other commands and then run those commands with the new aliases", "Allows the user to change the prompt the precedes the CWD to <arg1>",
							"History", "Refreshes the pathlist to the program's starting path. This command does not work properly and may cause undefined behavior.\n", "Lists this shell's built in commands and their functions", "Exits the shell", "Exits the shell" };
	
	char *prompt = calloc(PROMPTMAX, sizeof(char));				// The prompt printed before [cwd]> before each user input
	char *commandline = calloc(MAX_CANON, sizeof(char));		// The full string passed in by the user
	char *commandlineCONST = calloc(MAX_CANON, sizeof(char));	// Keeps a constant copy of the full commandline passed by the user
	char **args = calloc(MAXARGS, sizeof(char*));				// The arguments passed into the shell. args[0] is the command, args[size] = NULL
	char **memory = calloc(500, sizeof(char*));					// The saved command stack
	char **envMem = calloc(MAXMEM, sizeof(char*));				// Keeps a list of all the names of all env variables so that we can print them all using getenv()
	char **returnPtr = calloc(MAXMEM, sizeof(char*));			// Memory management helper
	char *mHelp = NULL;											// Memory management helper
	char *pathRtr = NULL;										// Memory management helper
	char *savedPath = getenv("PATH");							// Keeps a copy of the PATH env var during startup so later the user could theoretically reset it
	char command[2046];											// Just used right before executing a non-builtin to ensure the right string is passed into exec_command()
	bool go = true;												// Loop execution bool, keeps the program running until the user exits, terminates or causes problems
	bool clearedPath = false;									// Set to true whenever the user changes the PATH env variable until the pathlist is refreshed properly
	bool trigWild = false;										// Set to true during the loop if user passes in a wild arg. Used for print output cleanliness
	bool bg = false;											// Set to true while the current command is to be backgrounded
	bool firstUser = true;										// Set to true until a user is added to the watchuser list
	int argsc = 0;												// # of args passed to shell, including the command 
	int mem = 0;												// Used in history to keep track of how many commands to print
	int mems = 0;												// Used with history to keep track of how many commands have been saved
	int returns = 0;											// Used with the char** returnPtr to keep track of how many strings to free with plumber()
	int aliases = 0;											// Keeps track of # of aliases
	int h = 0;													// Keeps track of the position in the char** memory to add the next saved command at
	int status = 1;												// Stuff for fork()
	//int uid = 1;												// User ID?
	int features = 17;											// # of built in functions, used in where/which to loop through builtins[]
	//struct passwd *password_entry;								// Not too sure, let's just leave it here
	pathelement *pathlist = NULL;								// The struct that hold PATH info
	aliasEntry aliasList[100];									// Holds the list of aliases
	pid_t	pid = getpid();										// Keeps track of the parent process ID
	pid_t return_pid;											// Keeps track of backgrounded child PIDs
	//uid = getuid();											// User ID? 
	//password_entry = getpwuid(uid);         					// get passwd info
	pthread_mutex_unlock(&watchuser_lock);						// Mutex lock can start unlocked
	
	
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
			
			if (lastChar(commandline) == 0) 
			{
				bg = true;
				if (strlen(commandline) > 1) { commandline[strlen(commandline) - 1] = 0; } 
			}
			else { bg = false; }
			
			// Save a copy of this formatted commandline
			strcpy(commandlineCONST, commandline);
			
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
			}
		//	// WHICH : Prints out the first instance in 'PATH' of the input command
			else if ((strcmp(args[0], "which") == 0) || strcmp(args[0], "WHICH") == 0)
			{
				printf("Executing built-in which\n");
				
				// Checks out the pathlist to make sure things are set correctly
				// User may have edited it with 'setenv'
				if (!clearedPath) { pathlist = pathlist->head; }
				if (mHelp != NULL) { free(mHelp); }
				mHelp = refreshPath(pathlist);
				headRef(pathlist);
				clearedPath = false;
				
				// Now find the first instance of command and print it
				which(args[1], builtIns, args[2], features, pathlist);
			}
		//	// WHERE : Prints out every instance in 'PATH' of every input command
			else if ((strcmp(args[0], "where") == 0) || strcmp(args[0], "WHERE") == 0)
			{
				printf("Executing built-in where\n");
				
				// Checks out the pathlist to make sure things are set correctly
				// User may have edited it with 'setenv'
				if (!clearedPath) { pathlist = pathlist->head; }
				if (mHelp != NULL) { free(mHelp); }
				mHelp = refreshPath(pathlist);
				headRef(pathlist);
				clearedPath = false;
				
				// Now find all instances of command and print them
				where(args, pathlist, builtIns, features);
			}
		//	// CD : Changes directory	
			else if (strcmp(args[0], "cd") == 0)
			{
				printf("Executing built-in cd\n");
				changeDirectory(envp,args, argsc, envMem);
			}
		//	// PWD : Prints out the current directory
			else if ((strcmp(args[0], "pwd") == 0) || (strcmp(args[0], "PWD") == 0))
			{
				printf("Executing built-in pwd\n");
				printf("%s\n", getenv("CURDIR"));
			}
		//	// PROMPT : Allows the user to alter the 'prompt' variable
			else if ((strcmp(args[0], "prompt") == 0) || (strcmp(args[0], "PROMPT") == 0))
			{
				printf("Executing built-in prompt\n");
				prompter(args, prompt, argsc);
			}
		//	// PID : Prints out the PID of the shell
			else if ((strcmp(args[0], "pid") == 0) || (strcmp(args[0], "PID") == 0))
			{
				printf("Executing built-in pid\n");
				printf("pid = %jd\n", (intmax_t) pid);
			}
		//	// HISTORY : Prints out the last X commands entered, X is 10 or the entered number
			else if ((strcmp(args[0], "history") == 0) || (strcmp(args[0], "HISTORY") == 0) || (strcmp(args[0], "hist") == 0))
			{
				printf("Executing built-in history\n");
				hist(args, mem, memory, mems, argsc);
			}
		//	// LIST : Prints out all the files in the current directory or the passed in directories
			else if ((strcmp(args[0], "list") == 0) || (strcmp(args[0], "LIST") == 0))
			{
				printf("Executing built-in list\n");
				listHelper(argsc, getenv("CURDIR"), args);
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
			}
		//	// SETENV : Allows the user to alter the environment
			else if (strcmp(args[0], "setenv") == 0)
			{
				printf("Executing built-in setenv\n");
				
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
				
				// Reset home directory in case user changed it
				//homedir = getenv("HOME");
			}
		//	// ALIAS : Prints out all the aliases or allows the user to create one	
			else if ((strcmp(args[0], "alias") == 0) || (strcmp(args[0], "ALIAS") == 0))
			{
				printf("Executing built-in alias\n");
				aliases = proc_alias(aliasList, argsc, args, aliases);
			}
		//	// KILL : Kills a passed in PID or sends the passed in signal to it
			else if ((strcmp(args[0], "kill") == 0) || (strcmp(args[0], "KILL") == 0) || (strcmp(args[0], "DESTROY") == 0))
			{
				printf("Executing built-in kill\n");
				kill_proc(argsc, prompt, &memory, pathlist, commandlineCONST, &args, envMem, returnPtr, mHelp, mH, pathRtr, pid, aliases, aliasList);
			}
		//	// WATCHUSER : Allows the user to track a user and to be notified when a tracked user logs in
			else if ((strcmp(args[0], "watchuser") == 0) || (strcmp(args[0], "WATCHUSER") == 0) || (strcmp(args[0], "DESTROY") == 0))
			{
				printf("Executing built-in watchuser\n");
				pthread_t tid1;
				if (argsc == 2)
				{
					if (firstUser)
					{
						pthread_create(&tid1, NULL, watchuser, "Watchuser Thread");
						firstUser = false;
					}
					pthread_mutex_lock(&watchuser_lock);
					newUser(args[1], usersHead, usersTail);
					pthread_mutex_unlock(&watchuser_lock);
				}
				else if (argsc == 3)
				{
					if (strcmp(args[2], "off") == 0)
					{
						pthread_mutex_lock(&watchuser_lock);
						deleteUser(usersHead, args[1], usersHead, usersTail);
						pthread_mutex_unlock(&watchuser_lock);
					}
					else
					{
						printf("Improper usage of watchuser.\n");
					}
				}
				else if (argsc > 3)
				{
					printf("watchuser: too many arguments\n");
				}
				else 
				{
					userList *head1 = usersTail;
					while(head1!=NULL)
					{
						printf("%s\n", head1->node);
						head1=head1->prev;
					}
				}
			}
		//	// NEWLINE : Just sanity checking here I think	
			else if (strcmp(args[0], "\n") == 0) {}	

		//	// COMMANDS : Prints out all the built in commands of this shell
			else if (strcmp(args[0], "commands") == 0) 
			{
				for (int feat = 0; feat < features; feat++) { printf("%s ---:--- %s\n", builtIns[feat], descrips[feat]); }
			}
			
		//	// REFRESH PATH : sets the pathlist back to where it started when the shell started up
			// 								CURRENTLY DOES NOT FUNCTION
			else if (strcmp(args[0], "refreshpath") == 0) 
			{
				setenv("PATH", savedPath, 1);
				mH = get_path(&pathlist);
				headRef(pathlist);
			}		

		//	// DEBUG : During development was used to print various things and to check values during debugging
			else if (strcmp(args[0], "debug") == 0) 
			{
				
			}
			
		//	// PREV : Prints out the previous directory
			else if ((strcmp(args[0], "prev") == 0) || (strcmp(args[0], "previous") == 0))
			{
				printf("Executing built-in prev\n");
				//printf("%s\n", prev);
				printf("%s\n", getenv("PREVDIR"));
			}

			// END BUILT IN COMMANDS
			
			else 
			{
				// Checks out the pathlist to make sure things are set correctly
				// User may have edited it with 'setenv'
				if (!clearedPath) { pathlist = pathlist->head; }
				if (mHelp != NULL) { free(mHelp); }
				mHelp = refreshPath(pathlist);
				headRef(pathlist);
				clearedPath = false;
				
				// Ensure we're running the right thing here
				// Doesn't seem like this should be necessary, but doesn't work without it
				strcpy(command, args[0]);
				
				// Try to execute the command
				exec_command(command, commandlineCONST, args, envp, pid, pathlist, status, trigWild, bg); 
			}
			
			if (bg)
			{
				return_pid = waitpid(-1, &status, WNOHANG); // WNOHANG def'd in wait.h
				if (return_pid == -1) 
				{
					//error
					perror("Nonblocking waitpid");
				} 
				else if (return_pid == 0) 
				{
					// child is still running
				} 
				else if (return_pid == pid) 
				{
					// child is finished
					// status = exit status of child
					printf("Child done\n");
					bg = false;
				}
			}
			
			
			// Ensure the prompt always shows up before harvesting user input
			// Prevents what looks like the shell is hanging when really it is just waiting for input
			if (go) 
			{ 
				fprintf(stderr, "%s[%s]>", prompt, getenv("CURDIR")); 
				argsc = 0;
			}
			// Here is where we exit the user input loop if they have exited
			else { break; }
		}
	}
	
	// Free all allocated memory (ideally)
	plumber(prompt, &memory, pathlist, commandlineCONST, &args, envMem, returnPtr, mHelp, mH, pathRtr, true, aliases, aliasList);
	
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
 * @param command   		The array of commands given to search for
 * @param cont				Set to true during which when we want to just find the first instance, causes the loop to break early
 * @param print          	Set to false when searching for a command to execute, because we don't want to print it in that case
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
	
	
int lastChar(const char *str)
{
  if(*str && str[strlen(str + 1)] == '&')
	{ return 0; }
  else
	{ return 1; }
}

void *watchuser(void *param)
{
	//const char *name = param;
	struct utmpx *up;
	while(1){
    sleep(30);
	  setutxent();
	  while((up = getutxent() )){
	    if (up->ut_type == USER_PROCESS){
        pthread_mutex_lock(&watchuser_lock);
	    	userList *temp = usersTail;
	    	while(temp !=NULL){
	      	if (strcmp(temp->node, up->ut_user) == 0){
					 printf("%s had logged on %s from %s\n", up->ut_user, up->ut_line, up->ut_host);
	    		}
	    	  temp=temp->prev;
	    	}
        pthread_mutex_unlock(&watchuser_lock);
	    }
	  }
	}
}

