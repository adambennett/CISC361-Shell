#include "sh.h"

bool hasWildcards(char *commandline)
{
	for(int i=0; WILDCARDS[i] != '\0'; i++) { if(strchr(commandline, WILDCARDS[i]) != NULL){ return true; } }	
	return false;	
}

char **expand(char **args, int argc)
{
	char **expandedArgs = calloc(MAXMEM, sizeof(char*));
	char *ptr = NULL;
	char *temp;
	char *token;
	if (argc < 3)
	{
		expandedArgs[0] = strdup(args[0]);
		temp = expandArgs(args[1]);
		token = strtok_r(temp, " ", &ptr);
		expandedArgs[1] = strdup(token);
		for(int i = 2; token != NULL && i < MAXTOK; i++)
		{
			token = strtok_r(NULL, " ", &ptr);
			if (token != NULL) 
			{
				expandedArgs[i] = strdup(token);
			}
			else { expandedArgs[i] = NULL; }
		}		
	}
	else
	{
		expandedArgs[0] = strdup(args[0]);
		temp = expandArgs(args[argc - 1]);
		token = strtok_r(temp, " ", &ptr);
		for (int q = 1; q <= argc - 2; q++)
		{
			expandedArgs[q] = strdup(args[q]);
		}
		
		if (token != NULL) { expandedArgs[argc - 1] = strdup(token); }
		for(int k = argc; token != NULL && k < MAXTOK; k++)
		{
			token = strtok_r(NULL, " ", &ptr);
			if (token != NULL) 
			{
				expandedArgs[k] = strdup(token);
			}
			else { expandedArgs[k] = NULL; }
		}
		
		//args[0] = command
		//args[1] - args[argc - 2] = regular args
		//args[argc - 1] = wild arg
		//args[argc] = NULL
		
		//args[0] = command
		//args[1] - args[argc - 2] = regular args
		//args[argc - 1] - args[argc + #wild args] = wild args
		
		
		//int wildArgIndex = whichArgIsWild(args);
		//Put command in expArgs[0]
		//char **temp2 = saveNonWildArgs(args, wildArgIndex);
		//int tempSize = countEntries(temp2);
		//for (i = 0; i < tempSize; i++)
		//{
		//	fill expanded args from [1] to [i + 1]
		//}
		//temp = expandArgs(args[wildArgIndex]);
		//fill expandedArgs[i] to [i + new wild args] with new wild args
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
			//char* argcpy = malloc(strlen(arg) + 1);
			//strcpy(argcpy, arg);
			char *argcpy = strdup(arg);
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
		//char* argcpy = malloc(strlen(arg) + 1);
		char *argcpy = strdup(arg);

		//## Free up memory
		globfree(&pglob);

		//strcpy(argcpy, arg);
		return argcpy;
	}
}

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