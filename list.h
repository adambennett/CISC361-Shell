#ifndef list_h
#define list_h

#include "sh.h"

void list ( char *dir);
int listCheck(char *dir);
void listHelper(int q, char *owd, char **args);

#endif //list_h