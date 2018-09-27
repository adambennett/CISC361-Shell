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

void list ( char *dir)
{
	DIR* directory;
	struct dirent* entrypoint;
	directory = opendir(dir);
	if(directory == NULL) { printf("Can't open directory %s\n", dir); }
	else
	{
		while((entrypoint = readdir(directory)) != NULL) { printf("%s\n", entrypoint->d_name); }
		closedir(directory);
	}
} 


int listCheck(char *dir)
{
	DIR* directory;
	struct dirent* entrypoint;
	directory = opendir(dir);
	if(directory == NULL) { return 0; }
	else { return 1; } 
}

void listHelper(int q, char *owd, char **args)
{
	if (q == 1) { printf("%s: \n", owd); list(owd); }
	else
	{
		int i;
		for(i = 0; args[i] != NULL; i++)
		{
			if (listCheck(args[i]) > 0)
			{
				printf("\n");
				printf("%s: \n", args[i]);
				list(args[i]);
			}
			else
			{
				printf("\n");
				printf("Cannot open directory: %s\n", args[i]);
			}
		}
	}
}