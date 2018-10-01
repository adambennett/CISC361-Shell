#include "sh.h"

/** 
 * @brief Switches alias entered to corresponding command
 *
 * Changes the alias entered by the user to instead
 * be the command associated with that alias for simple
 * execution.
 *
 * @param command   		The command entered by the user
 * @param aliasList         The list of aliases
 * @param count				The number of aliases that exist when called
 * 
 * @return Returns 0 if the alias does not exist. Otherwise returns 1.
 */
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

/** 
 * @brief Determines if a string is an alias
 *
 * Checks all the aliases in the alias list to
 * see if the string passed in exists as an alias
 * already or not.
 *
 * @param string   		The string to look for in the alias list
 * @param aliasList    	The list of aliases to search through
 * @param value			Pass 1 if you want to compare the string against aliases, 2 to compare against commands of aliases
 * @param count			The number of aliases that exist when called
 * 
 * @return Returns true if a match is found, or false otherwise
 */
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

/** 
 * @brief Prints all aliases
 *
 * Prints every alias in the alias list, as well as
 * their corresponding commands.
 *
 * @param aliasList    	The list of aliases to print
 * @param count			The number of aliases that exist when called
 */
void print_aliases(aliasEntry aliasList[], int count)
{
	for (int i = 0; i < count; i++)  
	{ 
		printf("%s    %s\n", aliasList[i].alias, aliasList[i].command); 
	}
}

/** 
 * @brief Adds an alias to the list
 *
 * Used to add aliases to the list. Checks to see
 * if the alias to add already exists first. If it 
 * exists, this function changes the command associated
 * with it to now be the most recently inputted command.
 * If the alias does not exist in the list, it is appended
 * to the end of the list.
 *
 * @param newAlias   	The name (alias) of the new alias
 * @param aliasList    	The list of aliases to search through and add to
 * @param command		The command of the new alias
 * @param count			The number of aliases that exist when called
 * 
 * @return Returns the number of aliases in the list after execution.
 */
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

/** 
 * @brief Called from sh.c to run alias command
 *
 * Allows the user to print out the list of aliases
 * or to add/modify an alias entry.
 *
 * @param aliasList    	The list of aliases to search through
 * @param argc			The number of arguments passed into the shell
 * @param args			The arguments passed into the shell
 * @param count			The number of aliases that exist when called
 * 
 * @return If the user adds an alias, returns count + 1. If not, returns count.
 */
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