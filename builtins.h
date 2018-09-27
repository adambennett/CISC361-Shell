#ifndef built_h
#define built_h

#include "sh.h"

char **cd (char **args, char *pwd, char *owd, char *homedir, char **dirMem, int q);
char *prompter(char **args, char *prompt, int q);
int hist(char *command, char **args, int mem, char **memory, int mems, int q);
void kill_proc(char **args, int q);


#endif //built_h