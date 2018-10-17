#include "sh.h"

/** 
 * @brief Checks commandline for * and ?
 *
 * Checks the commandline until it finds ? or * or the end of the commandline.
 * Then returns a bool based on if one of those two characters exists or not.
 * If does not distinguish if one or both of these characters were found.
 * 
 * @param commandline		The string passed in by the user during the fgets() loop in sh.cabs
 *
 * @return Returns true if a * or ? is present in commandline. Returns false otherwise.
 */
bool hasWildcards(char *commandline)
{
	for(int i=0; WILDCARDS[i] != '\0'; i++) { if(strchr(commandline, WILDCARDS[i]) != NULL){ return true; } }	
	return false;	
}

/** 
 * @brief Creates an argument array after wildcard processing
 *
 * Calls expandArgs() on what is hopefully the argument containing
 * a wildcard, which is then used to create an array of arguments
 * that contains, command, regArg1, regArg2, .. regArgN, wildArg1, wildArg2, ... wildArgN, NULL
 *
 * NOTE: this probably only works if the argument containing a wild character is argument 2 or 3,
 * assuming the command is argument 1.
 * 
 * @param pathlist			The path to search on
 * @param command   		The array of commands given to search for
 * @param cont				Set to true during which when we want to just find the first instance, causes the loop to break early
 * @param print          	Set to false when searching for a command to execute, because we don't want to print it in that case
 *
 * @return Returns the new char** array of arguments that was created after running through glob() inside of expandArgs().
 */
char **expand(char **args, int argc)
{
	char **expandedArgs = calloc(MAXMEM, sizeof(char*));
	char *ptr = NULL;
	char *temp;
	char *token;
	if (argc < 3)
	{
		expandedArgs[0] = strdup(args[0]);				// ~3 bytes leak
		char *tempTemp = expandArgs(args[1]);
		temp = strdup(tempTemp);						// ~80 byte leak
		free(tempTemp);
		token = strtok_r(temp, " ", &ptr);
		expandedArgs[1] = strdup(token);				// ~8 bytes leak
		for(int i = 2; token != NULL && i < MAXTOK; i++)
		{
			token = strtok_r(NULL, " ", &ptr);
			if (token != NULL) { expandedArgs[i] = strdup(token); }		// ~72 bytes leak
			else { expandedArgs[i] = NULL; }
		}	
		free(temp);
	}
	else
	{
		expandedArgs[0] = strdup(args[0]);								// ~3 bytes leak
		char *tempTemp = expandArgs(args[argc - 1]);
		temp = strdup(tempTemp);										// ~5 bytes leak
		free(tempTemp);
		token = strtok_r(temp, " ", &ptr);
		for (int q = 1; q <= argc - 2; q++) { expandedArgs[q] = strdup(args[q]); }	// ~3 bytes leak
		if (token != NULL) { expandedArgs[argc - 1] = strdup(token); }				// ~5 bytes leak
		for(int k = argc; token != NULL && k < MAXTOK; k++)
		{
			token = strtok_r(NULL, " ", &ptr);
			if (token != NULL) { expandedArgs[k] = strdup(token); }					// ~10 bytes leak
			else { expandedArgs[k] = NULL; }
		}
		free(temp);
	}
	return expandedArgs;
}

/** 
 * @brief Expands a wildcard argument
 *
 * Modifies the input string to instead be the processed wildcard string.
 * Using glob() we filter out the wildcard and change the string to instead
 * be a list of files that match the criteria passed in. This string is used
 * in other functions to create an array of arguments that we eventually pass
 * into execv().
 * 
 * @param arg			The argument containing the wildcard that needs expanding
 *
 * @return Returns the expanded argument as a string for use in expand()
 */
char *expandArgs(char *arg)
{
	glob_t pglob;
	
	// If glob() returns successfully
	if(glob(arg, 0, NULL, &pglob) == 0)
	{
		// If we pass an argument that has no wildcard for some reason, just return itself
		if(pglob.gl_pathc == 0)
		{
			char *argcpy = strdup(arg);
			globfree(&pglob);
			return argcpy;
		}

		//Determine length of expanded argument
		int length = 0;
		for(int i=0; i < pglob.gl_pathc; i++) { length += strlen(pglob.gl_pathv[i]) + 1; }

		//Allocate space for the expanded argument
		char* expanded_arg = calloc(length, sizeof(char));
		
		//Create expanded argument by looping through pglob.gl_pathc (which will contain the file names we seek)
        for(int i=0; i < pglob.gl_pathc; i++)
		{
            if (i > 0) { strcat(expanded_arg, " "); }
            strcat(expanded_arg, pglob.gl_pathv[i]);
        } 
		
        globfree(&pglob);
        return expanded_arg;
	}
	
	//If glob() fails - may be due to: out of memory, read error, or no matches found
	//In any of these cases, we just want the normal argument back
	else
	{
		//Make a copy of the argument and return
		char *argcpy = strdup(arg);
		globfree(&pglob);
		return argcpy;
	}
}