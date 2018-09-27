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

void plumber(char *prompt, char *commandline, char *buf, char *owd, char *pwd, char *prev, char **dirMem, char **args, char **memory, 
struct pathelement *pathlist, int q, int mems, char *commandlineCONST)
{
	free(prompt);		free(commandline);	free(buf);
	free(owd);			free(pwd);			free(prev);
	free(commandlineCONST);
	free(dirMem[0]); 	free(dirMem[1]);	free(dirMem);
	for (int i = 0; i < q; i++) { free(args[i]); } free(args);
	for (int i = 0; i < mems; i++) { free(memory[i]); } free(memory); 
	pathPlumber(pathlist);
}


void pathPlumber(struct pathelement *head)
{
  struct pathelement *current = head;
  struct pathelement *temp;
  while(current->next != NULL)
  {
    temp = current;
    current = current->next;
    free(temp);
  }
  free(current);
}