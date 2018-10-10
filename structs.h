#ifndef structs_h
#define structs_h

#include "sh.h"

// 
/** 
 * @brief Keeps linked list of PATH 
 *
 * A linked list struct that is used to keep track of the PATH environment variable
 * 
 * @param element		The path as a string
 * @param head			The head of the linked list (used for easy traversal)
 * @param next			The next pathelement in the list
 */
typedef struct pathelement
{
  char *element;				// a dir in the path
  struct pathelement *head;		// pointer to the list head
  struct pathelement *next;		// pointer to next node
}pathelement;


/** 
 * @brief Used with 'alias'
 *
 * Customized struct used to keep track of all alias entries
 * 
 * @param alias			The alias entered by the user
 * @param command   	The command associated with this alias
 * @param ptr			Used to keep track of allocations for memory management
 */
typedef struct aliasEntry
{
  char *alias;					// The user entered command name
  char *command;				// The actual command associated with this alias
  struct aliasEntry *ptr;		// Keeps track of allocation of aliasEntries from addAlias()
}aliasEntry;


typedef struct userList
{
  char *node;			
  struct userList *next;	
  struct userList *prev;
  char *user;
  int watch;
}userList;

#endif //structs_h