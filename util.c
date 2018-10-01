#include "sh.h"

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

/** 
 * @brief Copies the environment to envMem
 *
 * Copies the name of every environment variable
 * to the saved array of names.
 * 			
 * @param envMem 	The array of all env variable namespace
 * @param envp		The envp containing all env variables
 */
void fillEnvMem(char **envMem, char **envp)
{
	int e = 0;
	for (e = 0; envp[e] != NULL; e++)
	{
		char *temp = malloc(strlen(envp[e]) + 1);
		strcpy(temp, envp[e]);
		envMem[e] = strtok(temp, "=");
	}
	
	envMem[e] = NULL;
}

/** 
 * @brief Prints a char**
 *
 * Loops through a char** and prints it out element by element
 * 			
 * @param array		Array to print
 */
void arrayPrinter(char **array)
{
	int e;
	for (e = 0; array[e] != NULL; e++) 
	{
		printf("[%d]: %s\n", e, array[e]);
	}
}

/** 
 * @brief Copies a char**
 *
 * Copies the second argument over into the first argument.
 * This function allocates space for the string copies.
 * 			
 * @param to 	Array to copy to
 * @param from	Array to copy from
 */
void copyArray(char **to, char **from)
{
	int e = 0;
	for (e = 0; from[e] != NULL; e++)
	{
		to[e] = malloc(strlen(from[e]) + 1);
		strcpy(to[e], from[e]);
	}
	
	to[e] = NULL;
}

/** 
 * @brief Copies a char** from the selected index
 *
 * Same as copyArray() but it starts copying at from[index]
 * 			
 * @param to 	Array to copy to
 * @param from	Array to copy from
 * @param index Index to start copying from
 */
void copyArrayIndexed(char **to, char **from, int index)
{
	int e = 0;
	int i = index;
	for (e = 0; from[i] != NULL; i++)
	{
		to[i] = malloc(strlen(from[e]) + 1);
		strcpy(to[i], from[i]);
	}
	
	to[i] = NULL;
}

