#include "sh.h"

int which(char *command, char **builtins, char *arg, int features, pathelement *pathlist)
{
	if (command == NULL) { printf("which: too few arguments\n"); return 0; }
	if (arg != NULL) { printf("which: too many arguments\n"); return 0; }
	int i = 0; 
	for (i = 0; i < features; i++) { if (strcmp(command, builtins[i]) == 0) { strcat(command, ": shell built-in command."); return 0; } }
	commandSet(pathlist, command, false, true);
	return 1;
}


void where(char **command, pathelement *pathlist, char **builtins, int features)
{
	if (command[1] == NULL) { printf("where: too few arguments\n"); }
	for (int j = 1; command[j] != NULL; j++)
	{
		int i = 0;
		for (i = 0; i < features; i++) { if (strcmp(command[j], builtins[i]) == 0) { printf("%s is a shell built-in\n", command[j]); } }
		commandFind(pathlist, command[j], true, true);
	}
} 

/** 
 * @brief cd helper function
 *
 * Allows the program to change directory simply with one function calloc
 * from sh.c, using only a few variables. cd returns a char**, the first string
 * represents the previous directory you came from after executing, and the second
 * string is the current directory.
 * 
 * @param args   			The array of arguments passed in with the cd command
 * @param pwd				Current working directory
 * @param owd          		Previous directory
 * @param homedir			Home directory
 * @param dirMem			The two string array that helps keep track of current/prev dir
 * @param argc				Number of arguments on commandline (includes cd)
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
 */
int prompter(char **args, char *prompt, int argc)
{
	char *buffer = calloc(MAX_CANON, sizeof(char));
	if (argc > 2) { printf("prompt: too many arguments.\n"); return 0; }
	else if (argc == 2) 
	{ 
		strcat(buffer, args[0]); strcat(buffer, " "); 
		//free(prompt);
		//prompt = malloc(strlen(buffer) + 1);
		strcpy(prompt, buffer);
		free(buffer);
		return 1;
	}
	else
	{
		printf("input prompt prefix:");
		fgets(buffer, BUFFER, stdin);
		buffer[strlen(buffer) - 1] = 0;
		strcat(buffer, " ");
		//free(prompt);
		//prompt = malloc(strlen(buffer) + 1);
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
 */
int hist(char **args, int mem, char **memory, int mems, int argc)
{
	if (argc > 2) { printf("history: too many arguments."); return 0; }
	else
	{
		if (args[0] != NULL)
		{
			int i = 0;
			mem = atoi(args[0]);
			if (mem > mems) { mem = mems; }
			for (i = 0; i < mem; i++) { printf("(%d): %s\n", i, memory[i]); }
			return 1;
		}
		else 
		{
			int i = 0;
			if (mem > 10) { mem = 10; }
			if ((mem < 10) && (mems > 10)) { mem = 10; }
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
void kill_proc(char **args, int argc, char *prompt, char *buf, char *owd, char *pwd, char *prev, 
			char **dirMem, char ***memory, pathelement *pathlist, 
			char *commandlineCONST,	char ***argsEx, char **envMem, char **returnPtr, char *memHelper,
			char *memHelper2, char *pathRtr, pid_t pid, int aliases, aliasEntry aliasList[])
{
	int termSignals[40] = {1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18, 21, 22, 24, 25, 26, 27, 30, 31 };
	if (argc == 1) { printf("Improper usage of kill.\n"); }
	else if (argc == 2) 
	{ 
		int temp = atoi(args[0]);
		if (temp == (intmax_t)pid)
		{
			plumber(prompt, buf, owd, pwd, prev, dirMem, args, memory, pathlist, commandlineCONST, argsEx, envMem, returnPtr, memHelper, memHelper2, pathRtr, false, aliases, aliasList);
			fclose(stdin);
			fclose(stdout);
			fclose(stderr);
		}
		kill(temp, SIGTERM);
	}
	else if(strstr(args[0], "-") != NULL) 
	{ 
		int temp = atoi(args[0]);
		int temp2 = atoi(args[1]);
		int signal = temp;
		signal = signal * -1;
		bool term = false;
		for (int i = 0; i < sizeof(termSignals) / sizeof(int); i++) 
		{ 
			if (termSignals[i] == signal) 
			{ 
				term = true;
				break;
			} 
		}
		if (term) 
		{  
			plumber(prompt, buf, owd, pwd, prev, dirMem, args, memory, pathlist, commandlineCONST, argsEx, envMem, returnPtr, memHelper, memHelper2, pathRtr, false, aliases, aliasList);
			fclose( stdin );
			fclose( stdout );
			fclose( stderr );
		}
		kill(temp2, signal); 
	}
}
