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
char *prompter(char **args, char *prompt, int argc)
{
	char *buffer = calloc(MAX_CANON, sizeof(char));
	if (argc > 2) { printf("prompt: too many arguments.\n"); return prompt; }
	else if (argc == 2) { strcat(buffer, args[0]); strcat(buffer, " "); return buffer; }
	else
	{
		printf("input prompt prefix:");
		fgets(buffer, BUFFER, stdin);
		buffer[strlen(buffer) - 1] = 0;
		strcat(buffer, " ");
		return buffer;
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
	if (argc > 2) { printf("history: too many arguments."); return mem; }
	else
	{
		if (args[0] != NULL)
		{
			int i = 0;
			mem = atoi(args[0]);
			if (mem > mems) { mem = mems; }
			for (i = 0; i < mem; i++) { printf("(%d): %s\n", i, memory[i]); }
			return mem;
		}
		else 
		{
			int i = 0;
			if (mem > 10) { mem = 10; }
			if ((mem < 10) && (mems > 10)) { mem = 10; }
			for (i = 0; i < mem; i++) { printf("(%d): %s\n", i, memory[i]); }
			return mem;
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
 */
void kill_proc(char **args, int argc)
{
	if (argc == 1) { printf("Improper usage of kill.\n"); }
	else if (argc == 2) { kill(atoi(args[0]), SIGTERM); }
	else if(strstr(args[0], "-") != NULL) { kill(atoi(args[1]), atoi(++args[0])); }
}