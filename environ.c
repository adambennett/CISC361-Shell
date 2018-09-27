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

char *get_pwd() 
{
  char *cwd = malloc(sizeof(char) * 100);
  if (getcwd(cwd,sizeof(cwd))!=NULL) { return cwd; }
  else { perror("get_pwd"); }
  return NULL;
}

void envprint(char **env, char **args, int q)
{
	int e = 0;
	char *temp;
	char temp2[1024] = "init";
	if (q > 2) { printf("printenv: Too many arguments.\n"); }
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



void envSet(char **args, char **env, struct pathelement *pathlist, int q)
{
	bool new = false;
	char *variable = findName(env, args[0]);
	int variables = countEntries(env);
	if (variable == NULL) { new = true; }
	
	if (q == 1)
	{
		envprint(env, args, q);
	}
	
	else if (q == 2)
	{
		if (new)
		{
			env = reinitEnv(env);
			newEnvVar(env, args[0], " ");
		}
		else { printf("Improper usage of setenv.\n"); }
		if (strcmp(args[0], "PATH") == 0) { pathPlumber(pathlist); pathlist = get_path(); }
	}
	
	else if (q == 3)
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

void newEnvVar(char **env, char *name, char *value)
{
	char *newVar = malloc(strlen(name) + strlen(value) + 2);
	sprintf(newVar, "%s=%s", name, value);
    putenv(newVar);
}

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


int countEntries(char **array)
{
	int i = 0;
	int count = 0;
	for (i = 0; array[i] != NULL; i++) { count++; }
	return count;
}




