#include "sh.h"

/** 
 * @brief Frees a linked list
 *
 * Properly frees the linked list pointed at by head
 * 			
 * @param head		Linked List to free
 */
void pathPlumber(pathelement *head)
{
  pathelement *current = head;
  pathelement *temp;
  while(current->next != NULL) { temp = current; current = current->next; free(temp); }
  free(current);
}

/** 
 * @brief Frees a char ** array
 *
 * Frees a char** array with (size) number elements
 * 			
 * @param array		Array to free
 * @param size		Size of array to free
 */
void arrayPlumber(char **array, int size) { for (int i = 0; i < size; i++) { free(array[i]); } free(array); }

/** 
 * @brief memory leak helper function
 *
 * Attempts to free as many allocated objects as possible to prevent as many memory leaks as possible.
 *
 * @param ...		These arguments are all different types, basically just a bunch of stuff allocated during sh.c
 */
void plumber(char *prompt, char *owd, char *pwd, char *prev, char **dirMem, char ***memory, 
			 pathelement *pathlist, char *commandlineCONST, char ***args, char **envMem,
			 char **returnPtr, char *memHelper, char *memHelper2, char *pathRtr, bool checker, int aliases, 
			 aliasEntry aliasList[])
{
	// Get sizes of char** arrays to pass into arrayPlumber()
	int mSize = countEntries(*memory); 
	int dSize = countEntries(dirMem); 
	int aeSize = countEntries(*args); 
	int eSize = countEntries(envMem);
	
	// Free all needed char** arrays
	arrayPlumber(*memory, mSize); 
	arrayPlumber(dirMem, dSize);
	if (checker) { arrayPlumber(*args, aeSize); }
	arrayPlumber(envMem, eSize);
	
	// Free char* arrays
	free(prompt);free(owd);free(pwd);free(prev);free(commandlineCONST);free(memHelper2);
	if (memHelper != NULL) { free(memHelper); }
	if (pathRtr != NULL) { free(pathRtr); }
	
	// Free returnPtr (used in setenv)
	if (returnPtr[0] != NULL) { int returnSize = countEntries(returnPtr); arrayPlumber(returnPtr, returnSize); }
	else { free(returnPtr); }
	
	// Free pathlist
	pathPlumber(pathlist);
	
	// Free alias list
	for (int i = 0; i < aliases; i++)
	{
		aliasEntry *alias = &aliasList[i];
		free(alias->alias);
		free(alias->command);
		free(alias->ptr);
	}
}