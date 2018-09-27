#ifndef sh_h
#define sh_h
#include "get_path.h"
#include "search.h"
#include "list.h"
#include "signal.h"
#include "builtins.h"
#include "plumbing.h"
#include "environ.h"

int pid;
int sh( int argc, char **argv, char **envp);
int execute(char *cmd, char **argv, char **env, pid_t pid);
int lineHandler(int *q, char ***args, char ***argv, char *commandline);

#define PROMPTMAX 32
#define MAXARGS 10
#define MAXMEM 100
#define BUFFER 128
#define MAXTOK 512

#endif //sh_h