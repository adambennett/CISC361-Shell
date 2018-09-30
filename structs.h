#ifndef structs_h
#define structs_h

#include "sh.h"

typedef struct pathelement
{
  char *element;				// a dir in the path
  struct pathelement *head;		// pointer to the list head
  struct pathelement *next;		// pointer to next node
}pathelement;

typedef struct aliasEntry
{
  char *alias;					// The user entered command name
  char *command;				// The actual command associated with this alias
  struct aliasEntry *ptr;		// Keeps track of allocation of aliasEntries from addAlias()
}aliasEntry;

#endif //structs_h