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
	if (argc  == 1) { for (e = 0; vars[e] != NULL; e++) { printf("%s\n", vars[e]); } }
	else if (argc == 2) { printf("%s\n", getenv(args[0])); }
	else { printf("printenv: Too many arguments.\n"); }
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
int envCheck(char **env, char **args, int argc)
{
	int e = 0;
	char *temp;
	char temp2[2046] = "init";
	bool found = false;
	if (argc > 2) { return 0; }
	else
	{
		if (args[0] == NULL) { return 1; }
		else 
		{
			for (e = 0; env[e] != NULL; e++)	
			{    
				strcpy(temp2, env[e]);
				temp = strtok(temp2, "=");
				if (strcmp(args[0], temp) == 0) { found = true; }
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
char *envSet(char **args, char **env, struct pathelement *pathlist, int argc, char **vars, bool freeEnvp, bool freePath)
{
	char *returnPtr = NULL;
	bool new = false;
	if (argc == 1) { envprint(env, args, argc, vars); return returnPtr; }
	else if (argc == 2)
	{
		char *variable = getenv(args[0]);
		if (variable == NULL) { new = true; }
		if (new) 
		{ 
			//reinitEnv(env, freeEnvp); 
			returnPtr = newEnvVar(env, args[0], " ", vars); 
		}
		else { printf("Improper usage of setenv.\n"); }
		if (strcmp(args[0], "PATH") == 0) { pathPlumber(pathlist); pathlist = get_path(); freePath = true; }
		return returnPtr;
	}
	else if (argc == 3)
	{ 
		//reinitEnv(env, freeEnvp); 
		returnPtr = newEnvVar(env, args[0], args[1], vars); 
		if (strcmp(args[0], "PATH") == 0) { pathPlumber(pathlist); pathlist = get_path(); freePath = true; } 
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
		vars[entries] = malloc(strlen(newVar) + 1); 
		strcpy(vars[entries], newVar); 
	}
	else
	{
		for (int i = 0; vars[i] != NULL; i++)
		{
			char *temp = malloc(strlen(vars[i]) + 1);
			strcpy(temp, vars[i]);
			char *temp2 = strtok(temp, "=");
			if (strcmp(temp2, name) == 0) 
			{ 
				vars[i] = malloc(strlen(newVar) + 1);
				strcpy(vars[i], newVar); 
			}
			free(temp);
		}
	}
	putenv(newVar);
	return newVar;
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
void reinitEnv(char **env, bool freeEnvp)
{
	int variables = countEntries(env);
	char **newEnv = calloc(variables * 2, sizeof(char*));
	char *oldVar;
	//int lastLen = strlen(env[0]) + 1;
	for (int i = 0; i < variables; i++)
	{
		oldVar = malloc(strlen(env[i]) + 1);
		sprintf(oldVar, "%s", env[i]); 
		newEnv[i] = malloc(strlen(oldVar) + 1);
		//newEnv[i] = oldVar;
		strcpy(newEnv[i], oldVar);
		free(oldVar);
		//lastLen = strlen(env[i]) + 1;
	}
	int ent = countEntries(env);
	if (freeEnvp) { arrayPlumber(env, ent); freeEnvp = false; }
	env = calloc((variables + 3), sizeof(char*));
	for (int i = 0; i < variables; i++)
	{
		//free(env[i]);
		env[i] = malloc(strlen(newEnv[i]) + 1);
		strcpy(env[i], newEnv[i]);
		//env[i] = newEnv[i];
		free(newEnv[i]);
	}
	env[variables] = NULL;
	free(newEnv);
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