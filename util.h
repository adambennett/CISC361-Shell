#ifndef util_h
#define util_h

#include "sh.h"

int countEntries(char **array);									//Counts elements in char** array
void fillEnvMem(char **envMem, char **envp);					//Copies the environment to envMem		
void arrayPrinter(char **array);								//Prints a char**		
void copyArray(char **to, char **from);							//Copies a char**
void copyArrayIndexed(char **to, char **from, int index);		//Copies a char** from the selected index

#endif //util_h