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

char *which(char *command, char **builtins, char *arg, int features, struct pathelement *pathlist, bool freePath)
{
	if (command == NULL) { return "which: too few arguments"; }
	if (arg != NULL) { return "which: too many arguments"; }
	bool found = false;
	int i = 0;
	pathlist = get_path();
	freePath = true;
	for (i = 0; i < features; i++) { if (strcmp(command, builtins[i]) == 0) { strcat(command, ": shell built-in command."); return command; } }
	while (pathlist) 
	{
		char str[256];
		strcpy(str, pathlist->element);
		strcat(str, "/");
		strcat(str, command);
		if (access(str, X_OK) == 0)
		{
			strcpy(command, str);
			found = true;
			break;
		}
		pathlist = pathlist->next;
	}
	if (found == false) { strcat(command, ": Command not found."); }
	return command;
}

char *quickwhich(char *command, struct pathelement *pathlist, bool freePath)
{
	bool found = false;
	pathlist = get_path();
	freePath = true;
	while (pathlist) 
	{
		char str[256];
		strcpy(str, pathlist->element);
		strcat(str, "/");
		strcat(str, command);
		if (access(str, X_OK) == 0)
		{
			strcpy(command, str);
			found = true;
			break;
		}
		pathlist = pathlist->next;
	}
	if (found == false) { return NULL; }
	return command;
} 

int where(char *command, struct pathelement *pathlist, char **builtins, int features, bool freePath)
{
	if (command == NULL) { printf("where: too few arguments\n"); return -2; }
	bool found = false;
	int i = 0;
	char str[2046];
	pathlist = get_path();
	freePath = true;
	for (i = 0; i < features; i++) { if (strcmp(command, builtins[i]) == 0) { printf("%s is a shell built-in\n", command); return 0; } }
	while (pathlist) 
	{
		strcpy(str, pathlist->element);
		strcat(str, "/");
		strcat(str, command);
		if (access(str, X_OK) == 0)
		{ found = true; printf("%s\n", str); }
		pathlist = pathlist->next;
	}
	if (found == false) { printf("%s: Command not found.\n", command); return -1; }
	else { return 1; }
} 