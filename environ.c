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
void envprint(char **env, char **args, int argc)
{
	int e = 0;
	char *temp;
	char temp2[1024] = "init";
	if (argc > 2) { printf("printenv: Too many arguments.\n"); }
	else
	{
		if (args[0] == NULL) { for (e = 0; env[e] != NULL; e++) { printf("%s\n", env[e]); } }
		else 
		{
			for (e = 0; env[e] != NULL; e++)	
			{    
				strcpy(temp2, env[e]);
				temp = strtok(temp2, "=");
				if (strcmp(args[0], temp) == 0) { char *variable = findName(env, args[0]); printf("%s\n", variable); }
			}
		}
	}
}

/** 
 * @brief Checks to make sure we should print out the statement "Executing printenv"
 *
 * Checks the 2nd argument passed in with printenv (if there is one),
 * and if that variable exists in the environment, print it.
 * 
 * Returns 1 if the variable we're checking exits, or 0 if it does not.
 * 			
 * @param env				Environment array
 * @param args   			The array of arguments passed in
 */
int envCheck(char **env, char **args)
{
	int e = 0;
	char *temp;
	char temp2[2046] = "init";
	bool found = false;
	if (args[1] != NULL) { return 0; }
	else
	{
		if (args[0] == NULL) { return 1; }
		else 
		{
			for (e = 0; env[e] != NULL; e++)	
			{    
				strcpy(temp2, env[e]);
				temp = strtok(temp2, "=");
				if (strcmp(args[0], temp) == 0)
				{ found = true; }
			}
			if (found == false) { return 0; }
			else { return 1; }
		}
	}
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
void envSet(char **args, char **env, struct pathelement *pathlist, int argc)
{
	bool new = false;
	char *variable = findName(env, args[0]);
	if (variable == NULL) { new = true; }
	
	if (argc == 1)
	{
		envprint(env, args, argc);
	}
	
	else if (argc == 2)
	{
		if (new)
		{
			env = reinitEnv(env);
			newEnvVar(env, args[0], " ");
		}
		else { printf("Improper usage of setenv.\n"); }
		if (strcmp(args[0], "PATH") == 0) { pathPlumber(pathlist); pathlist = get_path(); }
	}
	
	else if (argc == 3)
	{
		env = reinitEnv(env);
		newEnvVar(env, args[0], args[1]);
		if (strcmp(args[0], "PATH") == 0)
		{
			pathPlumber(pathlist);
			pathlist = get_path();
		}
	}
	
	else
	{
		printf("setenv: Too many arguments.\n");
	}
}

/** 
 * @brief environment checker function
 *
 * Allows the user to easily find an environment variable by name.
 * Returns the full "Name=Value" string if found, or NULL if not found.
 * 			
 * @param args   			The array of arguments passed in
 * @param env				Environment array
 * @param pathlist			Linked list containing PATH
 * @param argc				Number of arguments passed in (includes setenv)
 */
char *findName(char **envi, char *name)
{
	if (name == NULL) { return NULL;}
	int i = 0;
	for (i = 0; envi[i] != NULL; i++)
	{
		char *temp = malloc(strlen(envi[i]) + 1);
		char *temp2 = malloc(strlen(temp) + 1);
		strcpy(temp, envi[i]);
		temp2 = strtok(temp, "=");
		if (strcmp(name, temp2) == 0) { return envi[i];}
	}
	return NULL;
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
void newEnvVar(char **env, char *name, char *value)
{
	char *newVar = malloc(strlen(name) + strlen(value) + 2);
	sprintf(newVar, "%s=%s", name, value);
    putenv(newVar);
}

/** 
 * @brief Resizes the environment
 *
 * Reallocates the environment array to allow room
 * for new entries. This function takes the size
 * of the old env.(in variables) and doubles it.
 * Returns the new environment.
 * 			
 * @param env			Environment array
 */
char **reinitEnv(char **env)
{
	int variables = countEntries(env);
	char **newEnv = calloc(variables * 2, sizeof(char*));
	for (int i = 0; env[i] != NULL; i++)
	{
		char *oldVar = malloc(strlen(env[i]) + 1);
		sprintf(oldVar, "%s", env[i]);
		newEnv[i] = oldVar;
	}
	return newEnv;
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