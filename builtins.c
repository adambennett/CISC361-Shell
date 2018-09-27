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

char **cd (char **args, char *pwd, char *owd, char *homedir, char **dirMem, int q)
{
	char *prev = calloc(strlen(dirMem[0]) + 1, sizeof(char));
	if (q > 2) 
	{
		printf("cd: Too many arguments.\n");
		free(prev);
		return dirMem;
	}
	
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
			else
			{
				strcpy(prev, dirMem[0]);
				strcpy(dirMem[0], owd);
			}
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
		return dirMem;
	}
}

char *prompter(char **args, char *prompt, int q)
{
	
	char *buffer = calloc(MAX_CANON, sizeof(char));
	if (q > 2) 
	{
		printf("prompt: too many arguments.\n"); 
		return prompt;
	}
	
	else if (q == 2)
	{
		strcat(buffer, args[0]);
		strcat(buffer, " ");
		return buffer;
	}
	
	else
	{
		printf("input prompt prefix:");
		fgets(buffer, BUFFER, stdin);
		buffer[strlen(buffer) - 1] = 0;
		strcat(buffer, " ");
		return buffer;
	}
}

int hist(char *command, char **args, int mem, char **memory, int mems, int q)
{
	if (q > 2) { printf("history: too many arguments."); return mem; }
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
			for (i = 0; i < mem; i++) { printf("(%d): %s\n", i, memory[i]); }
			return mem;
		}
	}
}

void kill_proc(char **args, int q)
{
	if (q == 1) 
	{
		printf("Improper usage of kill.\n");
	}
	
	else if (q == 2)
	{
		kill(atoi(args[0]), SIGTERM);
	}
	else if(strstr(args[0], "-") != NULL)
	{
		kill(atoi(args[1]), atoi(++args[0]));
	}
}