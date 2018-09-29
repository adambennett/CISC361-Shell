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

int which(char *command, char **builtins, char *arg, int features, struct pathelement *pathlist)
{
	if (command == NULL) { printf("which: too few arguments\n"); return 0; }
	if (arg != NULL) { printf("which: too many arguments\n"); return 0; }
	int i = 0;
	for (i = 0; i < features; i++) { if (strcmp(command, builtins[i]) == 0) { strcat(command, ": shell built-in command."); return 0; } }
	commandSet(pathlist, command, false);
	return 1;
}

/*
void quickwhich(char *command, struct pathelement *pathlist)
{
	commandSet(pathlist, command, false);
} 
*/

int where(char *command, struct pathelement *pathlist, char **builtins, int features)
{
	if (command == NULL) { printf("where: too few arguments\n"); return 0; }
	int i = 0;
	for (i = 0; i < features; i++) { if (strcmp(command, builtins[i]) == 0) { printf("%s is a shell built-in\n", command); return 0; } }
	commandSet(pathlist, command, true);
	return 1;
} 

void commandSet(struct pathelement *pathlist, char *command, bool cont)
{
	bool found = false;
	char *str = malloc(256 * sizeof(char));
	struct pathelement *temp = pathlist->head;
	while (temp->next != NULL) 
	{
		strcpy(str, temp->element);
		strcat(str, "/");
		strcat(str, command);
		if (access(str, X_OK) == 0)
		{
			strcpy(command, str);
			found = true;
			printf("%s\n", command);
		}
		temp = temp->next;
		if (!cont && found) { break; }
	}
	if (found == false) 
	{ 
		strcat(command, ": Command not found."); 
		printf("%s\n", command);
	}
	free(str);
}

void printPathlist(struct pathelement *pathlist)
{
	struct pathelement *temp = pathlist->head;
	while (temp->next != NULL) 
	{
		printf("Element: %s\n", temp->element);
		temp = temp->next;
	}
}