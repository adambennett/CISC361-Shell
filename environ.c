#include "sh.h"

/** 
 * @brief printenv helper function
 *
 * Allows the program to easily print out the environment.
 * 			
 * @param env				Environment array
 * @param args   			The array of arguments passed in
 * @param argc				Number of arguments on commandline (includes printenv)
 */
void envprint(char **env, char **args, int argc, char **vars)
{
	int e = 0;
	if (argc  == 1) { for (e = 0; vars[e] != NULL; e++) { if (getenv(vars[e]) != NULL){printf("%s=%s\n", vars[e], getenv(vars[e]));}}}
	else if (argc == 2) { printf("%s\n", getenv(args[1])); }
	else { printf("printenv: Too many arguments.\n"); }
}

/** 
 * @brief setenv helper function
 *
 * Allows the program to easily set an environment variable (new or old).
 * 			
 * @param args   			The array of arguments passed in
 * @param env				Environment array
 * @param pathlist			Linked list containing PATH
 * @param argc				Number of arguments passed in (includes setenv)
 *
 * @return Returns a string that is used as a reference for later memory management.
 */
char *envSet(char **args, char **env, pathelement **pathlist, int argc, char **vars, char *pathRtr, bool clearedPath)
{
	char *returnPtr = NULL;
	bool new = false;
	if (argc == 1) { envprint(env, args, argc, vars); return returnPtr; }
	else if (argc > 3) { printf("setenv: Too many arguments.\n"); return returnPtr; }
	else 
	{
		if ((strcmp(args[1], "CURDIR") != 0) && ((strcmp(args[1], "PREVDIR") != 0)) && ((strcmp(args[1], "NOCLOB") != 0)))
		{
			if (argc == 2)
			{
				char *variable = getenv(args[1]);
				if (variable == NULL) { new = true; }
				if (new) { returnPtr = newEnvVar(env, args[1], " ", vars); }
				else { printf("Improper usage of setenv.\n"); }
				if (strcmp(args[1], "PATH") == 0) 
				{ 
					pathelement *newPath;
					pathPlumber(*pathlist); 
					pathRtr = get_path(&newPath); 
					clearedPath = true;
					newPath->head = newPath;
					headRef(newPath);
					*pathlist = newPath;
				}
				return returnPtr;
			}
			else if (argc == 3)
			{ 
				returnPtr = newEnvVar(env, args[1], args[2], vars); 
				if (strcmp(args[1], "PATH") == 0) 
				{ 
					pathelement *newPath;
					pathPlumber(*pathlist); 
					pathRtr = get_path(&newPath); 
					clearedPath = true;
					newPath->head = newPath;
					headRef(newPath);
					*pathlist = newPath;
				}
				return returnPtr;
			}
		}
		else
		{
			if ((strcmp(args[1], "CURDIR") == 0) || ((strcmp(args[1], "PREVDIR") == 0)))
			{
				printf("Permission to %s denied! Protected variable.\nUse the 'cd' command to change directory.\n", args[1]);
			}
			else 
			{
				printf("Permission to %s denied! Protected variable.\nUse the 'noclobber' command to toggle.\n", args[1]);
			}
		}
		return returnPtr;
	}
	
}

/** 
 * @brief Add/update enviroment variable
 *
 * Allows the user to easily add or update an enviroment
 * variable.
 * 			
 * @param env			Environment array
 * @param name			Name for new variable
 * @param value			Value of new variable
 *
 * @return Returns a string that is passed into envSet() and returned from there into sh.c in order to free allocated memory later.
 */
char *newEnvVar(char **env, char *name, char *value, char **vars)
{
	//char *s = name;
	//while (*s) { *s = toupper((unsigned char) *s); s++; }
	int entries = countEntries(vars);
	char *newVar = malloc(strlen(name) + strlen(value) + 2);
	sprintf(newVar, "%s=%s", name, value);
	if (getenv(name) == NULL) 
	{ 
		vars[entries] = malloc(strlen(name) + 1); 
		strcpy(vars[entries], name); 
	}
	
	putenv(newVar);
	return newVar;
}

/** 
 * @brief Puts PATH into a linked list
 *
 * This function takes a pathelement struct and
 * initializes it with elements that are filled
 * up with the parsed PATH environment variable.
 * 			
 * @param pathlist		The pathelement struct to save the PATH linked list to
 *
 * @return Returns a maintained reference to a string that we want to free later
 */
char *get_path(pathelement **pathlist)
{
	/* path is a copy of the PATH and p is a temp pointer */
	char *path, *p;

	/* tmp is a temp point used to create a linked list and pathlist is a
	 pointer to the head of the list */
	pathelement *tmp = NULL;
	*pathlist = NULL;

	p = getenv("PATH");	/* get a pointer to the PATH env var.
			   make a copy of it, since strtok modifies the
			   string that it is working with... */
	path = malloc((strlen(p)+1)*sizeof(char));	/* use malloc(3C) this time */
	strncpy(path, p, strlen(p));
	path[strlen(p)] = '\0';

	p = strtok(path, ":"); 	/* PATH is : delimited */
	do				/* loop through the PATH */
	{				/* to build a linked list of dirs */
		if ( !*pathlist )		/* create head of list */
		{
			tmp = calloc(1, sizeof(pathelement));
			*pathlist = tmp;
		}
		else			/* add on next element */
		{
			tmp->next = calloc(1, sizeof(pathelement));
			tmp = tmp->next;
		}
			tmp->element = p;	
			tmp->next = NULL;
	} while ( (p = strtok(NULL, ":")) );
	
	return path;
} /* end get_path() */

/** 
 * @brief Refreshes the pathlist
 *
 * After the user has changed the PATH env var, we need to reset the linked list.
 * This function retrieves the new PATH env var and fills the linked list back up
 * with it, and makes sure the list only contains that now.
 *
 * NOTE: Pathlist should have space allocated for the entries we are refreshing into it already,
 * this function does not allocate new space for new nodes or a new pathlist.
 * 			
 * @param pathlist		The pathelement struct to save the PATH linked list to
 *
 * @return Returns a maintained reference to a string that we want to free later
 */
char *refreshPath(pathelement *pathlist)
{
	char *p, *path;
	p = getenv("PATH");	
	path = malloc((strlen(p)+1));
	strncpy(path, p, strlen(p));
	path[strlen(p)] = '\0';
	p = strtok(path, ":"); 	
	do				
	{				
		pathlist->element = p;	
		pathlist = pathlist->next;
	} while ( (p = strtok(NULL, ":")) );
	
	
	return path;
}

/** 
 * @brief Fixes pathlist heads
 *
 * The way the pathelement linked list is setup is that every member
 * has a reference to the head node (for easy traversal). This function
 * loops through the pathelement passed in and sets the heads all equal
 * to what the head of the head node is. 
 *
 * NOTE: The head of the linked list passed into this function should be
 * initialized before calling it.
 * 			
 * @param pathlist		The pathelement struct to save the PATH linked list to
 */
void headRef(pathelement *pathlist)
{
	while (pathlist->next != NULL)
	{
		pathlist->next->head = pathlist->head;
		pathlist = pathlist->next;
	}
	
	pathlist = pathlist->head;
}