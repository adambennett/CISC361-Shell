#include "sh.h"

int morphAlias(char *command, aliasEntry aliasList[], int count)
{
	for (int i = 0; i < count; i++) 
	{
		if (strcmp(aliasList[i].alias, command) == 0)
		{
			char *temp = malloc(strlen(aliasList[i].command) + 1);
			strcpy(temp, aliasList[i].command);
			strcpy(command, temp);
			free(temp);
			return 1;
		}
	}
	return 0;
}	

bool isAlias(char *string, aliasEntry aliasList[], int value, int count)
{
	bool found = false;
	for (int i = 0; i < count; i++) 
	{
		// Loop through the alias list to find it
		if (value == 1)
		{
			if (strcmp(aliasList[i].alias, string) == 0)
			{
				found = true;
				return found;
			}
		}
		else if (value == 2)
		{
			if (strcmp(aliasList[i].command, string) == 0)
			{
				found = true;
				return found;
			}
		}
	}
	return found;
}	// Checks to see if a string is an existing alias


void print_aliases(aliasEntry aliasList[], int count)
{
	for (int i = 0; i < count; i++)  
	{ 
		printf("%s    %s\n", aliasList[i].alias, aliasList[i].command); 
	}
}


int addAlias(char *newAlias, char *command, aliasEntry aliasList[], int count)
{
	// Check if it already exists in the alias list
	if (isAlias(newAlias, aliasList, 1, count))
	{
		// If the new alias already has an entry under that name
		for (int i = 0; i < count; i++) 
		{
			// Loop through the alias list to find it
			if (strcmp(aliasList[i].alias, newAlias) == 0)
			{
				// Set the old command to the new value
				free(aliasList[i].command);
				aliasList[i].command = malloc(strlen(command) + 1);
				strcpy(aliasList[i].command, command);
				return count;
			}
		}
		return count;
	}
	else
	{
		// Otherwise just insert the alias at the end of the list
		// Create new aliasEntry
		aliasEntry *alias = malloc( sizeof( aliasEntry ));
		alias->alias = malloc(strlen(newAlias) + 1);
		alias->command = malloc(strlen(command) + 1);
		
		// Fill it with passed in values
		strcpy(alias->alias, newAlias);
		strcpy(alias->command, command);
		
		// Insert and increment the alias count
		aliasList[count] = *alias;
		aliasList[count].ptr = alias;
		count++;
		return count;
	}
}	// Intelligently adds new or existing alias to list of aliases


int proc_alias(aliasEntry aliasList[], int argc, char **args, int count)
{
	int newCount = count;
	if (argc == 1)
	{
		print_aliases(aliasList, count);
		return count;
	}
	else if (argc == 3)
	{
		newCount = addAlias(args[1], args[2], aliasList, count); 
		return newCount;
	}
	else { return count; }
}
