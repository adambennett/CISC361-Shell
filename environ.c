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
 * @brief printenv helper function
 *
 * Allows the program to easily print out the environment.
 * 			
 * @param env				Environment array
 * @param args   			The array of arguments passed in
 * @param argc				Number of arguments on commandline (includes printenv)
 */
void envprint(char **env, char **args, int argc, char **vars)
{
	int e = 0;
	if (argc  == 1) { for (e = 0; vars[e] != NULL; e++) { if (getenv(vars[e]) != NULL){printf("%s=%s\n", vars[e], getenv(vars[e]));}}}
	else if (argc == 2) { printf("%s\n", getenv(args[0])); }
	else { printf("printenv: Too many arguments.\n"); }
}

/** 
 * @brief setenv helper function
 *
 * Allows the program to easily set an environment variable (new or old).
 * 			
 * @param args   			The array of arguments passed in
 * @param env				Environment array
 * @param pathlist			Linked list containing PATH
 * @param argc				Number of arguments passed in (includes setenv)
 */
char *envSet(char **args, char **env, struct pathelement *pathlist, int argc, char **vars)
{
	char *returnPtr = NULL;
	bool new = false;
	if (argc == 1) { envprint(env, args, argc, vars); return returnPtr; }
	else if (argc == 2)
	{
		char *variable = getenv(args[0]);
		if (variable == NULL) { new = true; }
		if (new) { returnPtr = newEnvVar(env, args[0], " ", vars); }
		else { printf("Improper usage of setenv.\n"); }
		if (strcmp(args[0], "PATH") == 0) { pathPlumber(pathlist); pathlist = get_path(); }
		return returnPtr;
	}
	else if (argc == 3)
	{ 
		returnPtr = newEnvVar(env, args[0], args[1], vars); 
		if (strcmp(args[0], "PATH") == 0) { pathPlumber(pathlist); pathlist = get_path(); } 
		return returnPtr;
	}
	else { printf("setenv: Too many arguments.\n"); return returnPtr; }
}

/** 
 * @brief Add/update enviroment variable
 *
 * Allows the user to easily add or update an enviroment
 * variable.
 * 			
 * @param env			Environment array
 * @param name			Name for new variable
 * @param value			Value of new variable
 */
char *newEnvVar(char **env, char *name, char *value, char **vars)
{
	int entries = countEntries(vars);
	char *newVar = malloc(strlen(name) + strlen(value) + 2);
	sprintf(newVar, "%s=%s", name, value);
	if (getenv(name) == NULL) 
	{ 
		vars[entries] = malloc(strlen(name) + 1); 
		strcpy(vars[entries], name); 
	}
	
	putenv(newVar);
	return newVar;
	//else
	//{
		/*
		bool found = false;
		int i = 0;
		for (i = 0; vars[i] != NULL; i++)
		{
			char *temp = malloc(strlen(vars[i]) + 1);
 			strcpy(temp, vars[i]);
 			if (strcmp(temp, name) == 0) 
 			{ 
				found = true;
 				vars[i] = malloc(strlen(newVar) + 1);
 				strcpy(vars[i], newVar); 
 			}
			if (found) { free(temp); break; }
 			free(temp);
		}
		
		if (!found) { vars[i] = malloc(strlen(newVar) + 1); strcpy(vars[i], name); }
		*/
	//}
	
	
}

/** 
 * @brief Counts elements in char** array
 *
 * Returns the number of elements of the given char** array
 * 			
 * @param array 	Array to iterate over
 */
int countEntries(char **array)
{
	int i = 0;
	int count = 0;
	for (i = 0; array[i] != NULL; i++) { count++; }
	return count;
}

void fillEnvMem(char **envMem, char **envp)
{
	int e = 0;
	for (e = 0; envp[e] != NULL; e++)
	{
		char *temp = malloc(strlen(envp[e]) + 1);
		strcpy(temp, envp[e]);
		envMem[e] = strtok(temp, "=");
	}
	
	envMem[e] = NULL;
}

void copyArray(char **to, char **from)
{
	int e = 0;
	for (e = 0; from[e] != NULL; e++)
	{
		to[e] = malloc(strlen(from[e]) + 1);
		strcpy(to[e], from[e]);
	}
	
	to[e] = NULL;
}

void copyArrayIndexed(char **to, char **from, int index)
{
	int e = 0;
	int i = index;
	for (e = 0; from[i] != NULL; i++)
	{
		to[i] = malloc(strlen(from[e]) + 1);
		strcpy(to[i], from[i]);
	}
	
	to[i] = NULL;
}

void arrayPrinter(char **array)
{
	int e;
	for (e = 0; array[e] != NULL; e++) 
	{
		printf("[%d]: %s\n", e, array[e]);
	}
}