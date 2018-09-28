/* 
  get_path.h
  Ben Miller

*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* function prototype.  It returns a pointer to a linked list for the path
   elements. */
struct pathelement *get_path();

struct pathelement
{
  char *element;				/* a dir in the path */
  struct pathelement *head;		/* pointer to the list head */
  struct pathelement *next;		/* pointer to next node */
};

