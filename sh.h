#ifndef sh_h
#define sh_h

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "get_path.h"
#include "search.h"
#include "list.h"
#include "signal.h"
#include "builtins.h"
#include "plumbing.h"
#include "environ.h"
#include "execute.h"

int pid;
int sh( int argc, char **argv, char **envp);

#define PROMPTMAX 32
#define MAXARGS 10
#define MAXMEM 100
#define BUFFER 128
#define MAXTOK 512

#endif //sh_h