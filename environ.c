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
	else if (argc == 2) { printf("%s\n", getenv(args[0])); }
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
 */
char *envSet(char **args, char **env, pathelement **pathlist, int argc, char **vars, char *pathRtr, bool clearedPath)
{
	char *returnPtr = NULL;
	bool new = false;
	if (argc == 1) { envprint(env, args, argc, vars); return returnPtr; }
	else if (argc == 2)
	{
		char *variable = getenv(args[0]);
		if (variable == NULL) { new = true; }
		if (new) { returnPtr = newEnvVar(env, args[0], " ", vars); }
		else { printf("Improper usage of setenv.\n"); }
		if (strcmp(args[0], "PATH") == 0) 
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
		returnPtr = newEnvVar(env, args[0], args[1], vars); 
		if (strcmp(args[0], "PATH") == 0) 
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
	else { printf("setenv: Too many arguments.\n"); return returnPtr; }
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
 */
char *newEnvVar(char **env, char *name, char *value, char **vars)
{
	char *s = name;
	while (*s) { *s = toupper((unsigned char) *s); s++; }
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

void arrayPrinter(char **array)
{
	int e;
	for (e = 0; array[e] != NULL; e++) 
	{
		printf("[%d]: %s\n", e, array[e]);
	}
}

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

void headRef(pathelement *pathlist)
{
	while (pathlist->next != NULL)
	{
		pathlist->next->head = pathlist->head;
		pathlist = pathlist->next;
	}
	
	pathlist = pathlist->head;
}
