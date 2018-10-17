#ifndef structs_h
#define structs_h

#include "sh.h"

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
  char *element;					// Directory in the path
  struct pathelement *head;			// Pointer to the list head
  struct pathelement *next;			// Pointer to next node
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
  char *alias;						// The user entered command name
  char *command;					// The actual command associated with this alias
  struct aliasEntry *ptr;			// Keeps track of allocation of aliasEntries from addAlias()
}aliasEntry;

/** 
 * @brief Used with 'watchuser'
 *
 * Customized struct used to keep track of all watchuser entries
 * 
 * @param node			Name of user
 * @param next   		Pointer to next node
 */
typedef struct userList
{
  char *node;						// Username
  struct userList *next;			// Pointer to next node
}userList;

/** 
 * @brief Used with 'watchmail'
 *
 * Customized struct used to keep track of all watchmail entries
 * 
 * @param fileName		Name of file
 * @param thread 		The node's associated thread
 * @param next   		Pointer to next node
 */
typedef struct mailList 
{
    char *filename;            		// Path to the file being watched
    pthread_t thread;           	// Thread reference for each file
    struct mailList *next;  		// Pointer to next node
}mailList;

#endif //structs_h