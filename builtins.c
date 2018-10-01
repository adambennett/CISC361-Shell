#include "sh.h"

/** 
 * @brief which helper function
 *
 * Finds all the instances of command in pathlist and prints
 * each of them to the console.
 * 
 * @param command   		The array of commands given to search for
 * @param builtins          Used to check if any commands are builtin functions
 * @param arg				Used to check if the user has passed in >1 command to search for
 * @param features			Number of builtin features
 * @param pathlist			The path to search on
 * 
 * @return Returns 0 if you pass the wrong amount of arguments, or if the command you're searching for is builtin.
 *		   Otherwise returns 1.
 */
int which(char *command, char **builtins, char *arg, int features, pathelement *pathlist)
{
	// Check to ensure the user is passing the right arguments
	if (command == NULL) { printf("which: too few arguments\n"); return 0; }
	if (arg != NULL) { printf("which: too many arguments\n"); return 0; }
	
	// Check to ensure the command we are searching for is not a builtin function
	int i = 0; 
	for (i = 0; i < features; i++) { if (strcmp(command, builtins[i]) == 0) { strcat(command, ": shell built-in command."); return 0; } }
	
	// If everything is ok above, find that command and return 1
	commandSet(pathlist, command, false, true);
	return 1;
}

/** 
 * @brief where helper function
 *
 * Finds all the instances of command in pathlist and prints
 * each of them to the console.
 * 
 * @param command   		The array of commands given to search for
 * @param pathlist			The path to search on
 * @param builtins          Used to check if any commands are builtin functions
 * @param features			Number of builtin features
 */
void where(char **command, pathelement *pathlist, char **builtins, int features)
{
	bool builtin = false;
	int argc = countEntries(command);
	
	// Check to ensure the user is passing the right arguments
	if (command[1] == NULL) { printf("where: too few arguments\n"); }
	
	// Loop through all the arguments passed to the 'where' command
	for (int j = 1; command[j] != NULL; j++)
	{
		// Console printing logic
		builtin = false;
		if (argc > 2) { printf("%s:\n", command[j]); }
		
		// Check to make sure each argument is not a builtin
		int i = 0;
		for (i = 0; i < features; i++) { if (strcmp(command[j], builtins[i]) == 0) { builtin = true; printf("%s is a shell built-in\n", command[j]); break; } }
		
		// Check to make sure we didn't just print out "this is a built in"
		// If not, find the command's path and print it
		if (!builtin) { commandFind(pathlist, command[j], true, true); }
		if (argc > 2) { printf("\n"); }		// More print logic
	}
} 

/** 
 * @brief cd helper function
 *
 * Allows the program to change directory simply with one function calloc
 * from sh.c, using only a few variables. cd 
 * 
 * @param args   			The array of arguments passed in with the cd command
 * @param pwd				Current working directory
 * @param owd          		Previous directory
 * @param homedir			Home directory
 * @param dirMem			The two string array that helps keep track of current/prev dir
 * @param argc				Number of arguments on commandline (includes cd)
 *
 * @return Returns a char**, the first string represents the previous directory you came 
 *         from after executing, and the second string is the current directory.
 */
char **cd (char **args, char *pwd, char *owd, char *homedir, char **dirMem, int argc)
{
	char *prev = calloc(strlen(dirMem[0]) + 1, sizeof(char));
	if (argc > 2) { printf("cd: Too many arguments.\n"); free(prev); return dirMem; }
	else
	{
		if (args[0] == NULL) 
		{
			free(dirMem[0]);
			dirMem[0] = calloc(strlen(owd) + 1, sizeof(char));
			strcpy(dirMem[0], owd);
			if (chdir(homedir) != 0) { perror("cd"); }
		} 
		else if (strcmp(args[0], "-") == 0)
		{
			strcpy(prev, dirMem[0]);
			if (chdir(prev) != 0) { perror("cd"); }
			else { strcpy(prev, dirMem[0]); strcpy(dirMem[0], owd); }
		}
		
		else 
		{
			if (chdir(args[0]) != 0) { perror("cd"); }
			else
			{
				free(dirMem[0]);
				dirMem[0] = calloc(strlen(owd) + 1, sizeof(char));
				strcpy(dirMem[0], owd);
			}
		}
	
		if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL ) { perror("getcwd"); exit(2); }
		free(dirMem[1]);
		dirMem[1] = calloc(strlen(pwd) + 1, sizeof(char));
		strcpy(dirMem[1], pwd);
		free(prev);
		free(pwd);
		return dirMem;
	}
}

/** 
 * @brief list command helper function
 *
 * Prints out the list of files in the given directory
 * 
 * @param dir   			The directory to open and list
 */
void list ( char *dir)
{
	DIR* directory;
	struct dirent* entrypoint;
	directory = opendir(dir);
	if(directory == NULL) { printf("Can't open directory %s\n", dir);}
	else
	{
		while((entrypoint = readdir(directory)) != NULL) { printf("%s\n", entrypoint->d_name); }
		closedir(directory);
	}
} 

/** 
 * @brief prompt helper function
 *
 * Allows the program to easily change the prompt that is printed in
 * front of the current directory each time the program prompts for input.
 * 
 * @param args   			The array of arguments passed in with the prompt command
 * @param prompt			The string that is used to display the prompt
 * @param argc				Number of arguments on commandline (includes prompt)
 *
 * @return Returns 0 if the user enters too many arguments. Returns 1 if the user sets
		   the string directly. Returns 2 if the user simply enters 'prompt'.
 */
int prompter(char **args, char *prompt, int argc)
{
	char *buffer = calloc(MAX_CANON, sizeof(char));
	
	// The user can only enter 1 other argument with 'prompt'
	if (argc > 2) { printf("prompt: too many arguments.\n"); return 0; }
	
	// If they enter prompt and 1 argument
	else if (argc == 2) 
	{ 
		// The user gave us the string so just set prompt = input
		strcat(buffer, args[0]); strcat(buffer, " "); 
		strcpy(prompt, buffer);
		
		// Free the input buffer after copying
		free(buffer);
		return 1;
	}
	
	// If they just enter prompt
	else
	{
		// We don't have a string so ask the user for it
		printf("input prompt prefix:");
		fgets(buffer, BUFFER, stdin);
		
		// Then do the same thing as if they passed it originally
		buffer[strlen(buffer) - 1] = 0;
		strcat(buffer, " ");
		strcpy(prompt, buffer);
		free(buffer);
		return 2;
	}
}

/** 
 * @brief history helper function
 *
 * Allows the program to easily print out the commandline history.
 * 			
 * @param args   			The array of arguments passed in with the history command
 * @param mem				Keeps track of where in the memory array we currently are at
 * @param memory			Where commandlines are saved
 * @param mems				The number of commandlines saved so far
 * @param argc				Number of arguments on commandline (includes history)
 *
 * @return Returns 0 if the user enters too many arguments. Returns 1 if the user prints out
 *		   a specified number of commands. Returns 2 if the user simply enters 'history'.
 */
int hist(char **args, int mem, char **memory, int mems, int argc)
{
	// Ensure the user is passing in the right arguments
	if (argc > 2) { printf("history: too many arguments."); return 0; }
	else
	{
		// If they pass history and 1 integer arg
		if (args[0] != NULL)
		{
			// Make sure argument is an integer
			int i = 0;
			mem = atoi(args[0]);	
			
			// If the argument passed in is greater than the number of saved commands
			// Set the amount of commands printed equal to the number of saved commands
			if (mem > mems) { mem = mems; }
			
			// Print saved commands until you get to the desired amount
			for (i = 0; i < mem; i++) { printf("(%d): %s\n", i, memory[i]); }
			return 1;
		}
		
		// If they just pass history
		else 
		{
			// This logic ensures that the amount of commands printed is always 10 in this case
			int i = 0;
			if (mem > 10) { mem = 10; }						// In case >10 commands are saved
			if ((mem < 10) && (mems > 10)) { mem = 10; }	// In case you printed <10 commands previously but >10 commands are saved
			
			// Now print out 10 commands
			for (i = 0; i < mem; i++) { printf("(%d): %s\n", i, memory[i]); }
			return 2;
		}
	}
}

/** 
 * @brief kill helper function
 *
 * Allows the program to easily kill processes.
 * 			
 * @param args   			The array of arguments passed in with the kill command
 * @param argc				Number of arguments on commandline (includes kill)
 * @param ...				The rest of the arguments are all just being passed in from the shell so we can free them if the program is killed
 */
void kill_proc(char **args, int argc, char *prompt, char *owd, char *pwd, char *prev, 
			char **dirMem, char ***memory, pathelement *pathlist, 
			char *commandlineCONST,	char ***argsEx, char **envMem, char **returnPtr, char *memHelper,
			char *memHelper2, char *pathRtr, pid_t pid, int aliases, aliasEntry aliasList[])
{
	// A list of all the termination signals a user could send to a process
	int termSignals[40] = {1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18, 21, 22, 24, 25, 26, 27, 30, 31 };
	
	// Ensure the user is passing in the right # of arguments
	if (argc == 1) { printf("Improper usage of kill.\n"); }
	
	// If the user enters kill and a process ID
	else if (argc == 2) 
	{ 
		// In this case, just make sure the pid passed is the parent process ID
		int temp = atoi(args[0]);
		if (temp == (intmax_t)pid)
		{
			// If it is, we're about to terminate so free up memory and close file descriptors
			plumber(prompt, owd, pwd, prev, dirMem, args, memory, pathlist, commandlineCONST, argsEx, envMem, returnPtr, memHelper, memHelper2, pathRtr, false, aliases, aliasList);
			fclose(stdin);
			fclose(stdout);
			fclose(stderr);
		}
		
		// TERMINATE
		kill(temp, SIGTERM);
	}
	
	// If the user enters 'kill -X pid' where X is an integer signal reference
	else if(strstr(args[0], "-") != NULL) 
	{
		// In this case we need to correct the signal input
		int temp = atoi(args[0]);
		int temp2 = atoi(args[1]);
		int signal = temp;
		signal = signal * -1;		// -X is just interpreted as a neg number by atoi(), so positive-fy it
		bool term = false;
		
		// Check if the signal is a termination signal
		for (int i = 0; i < sizeof(termSignals) / sizeof(int); i++) 
		{ 
			if (termSignals[i] == signal) 
			{ 
				term = true;
				break;
			} 
		}
		
		// If we have a termination signal and we're about to send it to the parent process
		if ((term) && (temp == (intmax_t)pid)) 
		{  
			// Free memory and close file descriptors in prepartion of termination
			plumber(prompt, owd, pwd, prev, dirMem, args, memory, pathlist, commandlineCONST, argsEx, envMem, returnPtr, memHelper, memHelper2, pathRtr, false, aliases, aliasList);
			fclose( stdin );
			fclose( stdout );
			fclose( stderr );
		}
		
		// TERMINATION (maybe)
		kill(temp2, signal); 
	}
}