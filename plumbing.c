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
 * @brief Frees a linked list
 *
 * Properly frees the linked list pointed at by head
 * 			
 * @param head		Linked List to free
 */
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

/** 
 * @brief Frees a char ** array
 *
 * Properly frees a char** array with (size) number elements
 * 			
 * @param array		Array to free
 * @param size		Size of array to free
 */
void arrayPlumber(char **array, int size)
{
	for (int i = 0; i < size; i++) { free(array[i]); }
	free(array);
}

/** 
 * @brief memory leak helper function
 *
 * Attempts to free as many allocated objects as possible to prevent memory leaks
 * as best as possible.
 * 			
 */
void plumber(char *prompt, char *commandline, char *buf, char *owd, char *pwd, char *prev, char **dirMem, char **args, char ***memory, 
struct pathelement *pathlist, int argc, int mems, char *commandlineCONST, char *tempHome, char *command, char ***argsEx)
{
	int aSize = countEntries(args);
	int mSize = countEntries(*memory);
	int dSize = countEntries(dirMem);
	int aeSize = countEntries(*argsEx);
	
	arrayPlumber(args, aSize);
	arrayPlumber(*memory, mSize);
	arrayPlumber(dirMem, dSize);
	arrayPlumber(*argsEx, aeSize);
	
	free(prompt);		
	//free(commandline);	
	free(buf);
	free(owd);			
	free(pwd);			
	free(prev);
	free(tempHome);
	free(command);
	free(commandlineCONST);	
	
	pathPlumber(pathlist);
	
}