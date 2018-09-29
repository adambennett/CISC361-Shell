#ifndef built_h
#define built_h

#include "sh.h"

char **cd (char **args, char *pwd, char *owd, char *homedir, char **dirMem, int argc);
int prompter(char **args, char *prompt, int argc);
int hist(char **args, int mem, char **memory, int mems, int argc);
void kill_proc(char **args, int argc);


#endif //built_h