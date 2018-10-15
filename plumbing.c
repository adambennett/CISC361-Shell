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
void plumber(char *prompt, char ***memory, 
			 pathelement *pathlist, char *commandlineCONST, char ***args, char **envMem,
			 char **returnPtr, char *memHelper, char *memHelper2, char *pathRtr, bool checker, int aliases, 
			 aliasEntry aliasList[], bool firstUser, pthread_t tid1, mailList *mailHead)
{
	// Get sizes of char** arrays to pass into arrayPlumber()
	int mSize = countEntries(*memory); 
	int aeSize = countEntries(*args); 
	int eSize = countEntries(envMem);
	
	// Free all needed char** arrays
	arrayPlumber(*memory, mSize); 
	if (checker) { arrayPlumber(*args, aeSize); }
	arrayPlumber(envMem, eSize);
	
	// Free char* arrays
	free(prompt);free(commandlineCONST);free(memHelper2);
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
	
	// Kill threads if there are any
	// Watchuser Thread
	if (!firstUser)
	{
		printf("Watchuser thread:\n");
		if (pthread_kill(tid1, SIGTERM) != 0)
		{
			perror("thread kill");
		}
	}
	
	// Watchmail Thread(s)
	/*
	mailList *temp = mailHead;
	if (temp != NULL) 
	{
		if (temp->next == NULL) { printf("Watchmail thread for %s:\n", temp->filename); if (pthread_kill(temp->thread, SIGTERM) != 0) { perror("thread kill"); } }
		while (temp->next != NULL)
		{
			printf("Watchmail thread for %s:\n", temp->filename);
			if (pthread_kill(temp->thread, SIGTERM) != 0) { perror("thread kill"); }
			temp = temp->next;
			if (temp->next == NULL) { printf("Watchmail thread for %s:\n", temp->filename); if (pthread_kill(temp->thread, SIGTERM) != 0) { perror("thread kill"); } }
		}
	}
	*/
}