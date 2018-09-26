
#include "get_path.h"

int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, char **builtins, char *arg, int features, struct pathelement *pathlist);
char *quickwhich(char *command, struct pathelement *pathlist);
int where(char *command, struct pathelement *pathlist, char **builtins, int features);
void list ( char *dir);
int listCheck(char *dir);
void listHelper(int q, char *owd, char **args);
char **cd (char **args, char *pwd, char *owd, char *homedir, char **dirMem);
char *prompter(char **args, char *prompt, int q);
int hist(char *command, char **args, int mem, char **memory, int mems);
void envprint(char **env, char **args);
int envCheck(char **env, char **args);
char **envSet(char **args, char **env, struct pathelement *pathlist, int q);
void printenv(char **envp);
char *get_pwd();
void plumber(char *prompt, char *commandline, char *buf, char *owd, char *pwd, char *prev, char **dirMem, char **args, 
char **memory, struct pathelement *pathlist, int q, int mems, char *commandlineCONST, char **argsEx);
void pathPlumber(struct pathelement *pathlist);
void sigintHandler(int sig_num);
void signalSTPHandler(int sig_num);
char *findName(char **envp, char *name);
int countEntries(char **array);
int execute(char *cmd, char **argv, char **env, pid_t pid);
int lineHandler(int *q, char ***args, char ***argv, char *commandline);
void kill_proc(char **args, int q);
//int findPosition(char **envi, char *name);
//void set_env(char *name, char *value, char **env);

#define PROMPTMAX 32
#define MAXARGS 10
#define MAXMEM 100
#define BUFFER 128
#define MAXTOK 512
