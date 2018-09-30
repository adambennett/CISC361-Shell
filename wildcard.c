#include "sh.h"

bool hasWildcards(char *commandline)
{
	for(int i=0; WILDCARDS[i] != '\0'; i++) { if(strchr(commandline, WILDCARDS[i]) != NULL){ return true; } }	
	return false;	
}

char **expand(char **args, int argc)
{
	char **expandedArgs = (char**)calloc(argc, sizeof(char*));
	char *ptr = NULL;
	char *temp;
	char *token;
	if (argc < 3)
	{
		expandedArgs[0] = args[0];
		temp = expandArgs(args[1]);
		token = strtok_r(temp, " ", &ptr);
		expandedArgs[1] = malloc(strlen(token) + 1);
		strcpy(expandedArgs[1], token);
		for(int i = 2; token != NULL && i < MAXTOK; i++)
		{
			token = strtok_r(NULL, " ", &ptr);
			if (token != NULL) 
			{
				expandedArgs[i] = malloc(strlen(token) + 1);
				strcpy(expandedArgs[i], token);
			}
			else { expandedArgs[i] = NULL; }
		}		
	}
	else
	{
		//int wildArgIndex = whichArgIsWild(args);
	}
	
	//int aSize = countEntries(args);
	//arrayPlumber(args, aSize);
	return expandedArgs;
}

char *expandArgs(char *arg)
{
	glob_t pglob;
	if(glob(arg, 0, NULL, &pglob) == 0)
	{
		//## If no wildcard in the argument return a copy of itself
		if(pglob.gl_pathc == 0)
		{
			char* argcpy = malloc(strlen(arg) + 1);
			strcpy(argcpy, arg);
			globfree(&pglob);		// Free up memory
			return argcpy;
		}

		//## Determine total length of expanded argument
		int length = 0;
		for(int i=0; i < pglob.gl_pathc; i++) { length += strlen(pglob.gl_pathv[i]) + 1; }

		//## Allocate new space for the expanded argument
		char* expanded_arg = calloc(length, sizeof(char));
		
		//## Form expanded argument string
        for(int i=0; i < pglob.gl_pathc; i++)
		{
            if (i > 0) { strcat(expanded_arg, " "); }
            strcat(expanded_arg, pglob.gl_pathv[i]);
        } 

        //## Free up memory 
        globfree(&pglob);

        return expanded_arg;
		
	}
	else
	{
		//## Make a copy of the argument and return
		char* argcpy = malloc(strlen(arg) + 1);

		//## Free up memory
		globfree(&pglob);

		strcpy(argcpy, arg);
		return argcpy;
	}
}

//char **saveNonWildArgs(char **args)
//{
	//char **memory = calloc(MAXMEM, sizeof(char*));
	//int aSize = countEntries(args);
	
//}

int whichArgIsWild(char **args)
{
	int aSize = countEntries(args);
	for (int i = 0; i < aSize; i++)
	{
		if (hasWildcards(args[i])) 
		{
			return i;
		}
	}
	
	return -1;
}

int howManyNewArgs(char **args, int indexOfWildArg)
{
	int count = 0;
	char* ptr = NULL;
	//*expandedArgs[0] = args[0];
	char *temp = expandArgs(args[indexOfWildArg]);
	char *token = strtok_r(temp, " ", &ptr);
	count++;
	//*expandedArgs[1] = token;
	for(int i = 1; token != NULL && i < MAXTOK; i++)
	{
		token = strtok_r(NULL, " ", &ptr);
		count++;
		//(*expandedArgs)[i] = token;
	}	
	return count;
}