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
#include "sh.h"

int sh( int argc, char **argv, char **envp )
{
	char *builtIns[2048] = {"cd", "which", "exit", "where", "pwd", "history", "pid", "kill", "list", "printenv", "setenv", "alias", "prompt", "hist", "quit"};
	char *prompt = calloc(PROMPTMAX, sizeof(char));
	char *commandline = calloc(MAX_CANON, sizeof(char));
	char *commandlineCONST = calloc(MAX_CANON, sizeof(char));
	char *buf = calloc(MAX_CANON, sizeof(char));
	char *command, *arg, *commandpath, *p, *pwd, *owd, *homedir, *prev;
	char **args = calloc(MAXARGS, sizeof(char*));
	char **argsEx = calloc(MAXARGS, sizeof(char*));
	char **memory = calloc(MAXMEM, sizeof(char*));
	char **dirMem = calloc(MAXARGS, sizeof(char*));
	char **env = envp;
	bool go = true;
	int uid, i, status, argsct = 1;
	int q, j, h, e = 0;
	int features = 15;						// number of built in functions
	int mem = 0;
	int mems = 0;
	size_t n = 1;
	struct passwd *password_entry;
	struct pathelement *pathlist;
	pid_t	pid = getpid();
	uid = getuid();
	password_entry = getpwuid(uid);         // get passwd info
	homedir = password_entry->pw_dir;		// Home directory to start out with

	 
	if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
	{
		perror("getcwd");
		exit(2);
	}
	owd = calloc(strlen(pwd) + 1, sizeof(char));
	prev = calloc(strlen(pwd) + 1, sizeof(char));
	dirMem[0] = malloc(sizeof(char*));
	dirMem[1] = malloc(sizeof(char*));
	memcpy(owd, pwd, strlen(pwd));
	memcpy(prev, owd, strlen(pwd));
	memcpy(dirMem[0], owd, sizeof(char));
	memcpy(dirMem[1], owd, sizeof(char));
	
	prompt[0] = ' '; prompt[1] = '\0';
	pathlist = get_path();					// Put PATH into a linked list

	while (go)
	{
		signal(SIGINT, sigintHandler);
		signal(SIGTSTP, signalSTPHandler);
		if (go) { fprintf(stderr, "%s[%s]>", prompt, owd); }
		while ((fgets(commandline, MAX_CANON, stdin) != NULL) && go) 
		{
			
			if (commandline[strlen(commandline) - 1] == '\n')
			{
				if (strlen(commandline) > 1) 
				{
					commandline[strlen(commandline) - 1] = 0; 	// replace newline with null
				} 
			}
			
			strcpy(commandlineCONST, commandline);
			int line = lineHandler(&q, &argsEx, &args, commandline);
			if (line == 1) 
			{
				memory[h] = calloc(strlen(commandlineCONST) + 1, sizeof(char));
				strcpy(memory[h], commandlineCONST);
				h++; mems++;
				if (mem < 10) { mem++; }
				if (mem > 10) { mem = 10; }
			}
			
			command = realloc(command, (size_t) (strlen(argsEx[0]) + 1) * sizeof(char));
			strcpy(command, argsEx[0]);
			
			// BUILT IN COMMANDS //
			if ((strcmp(command, "exit") == 0) || (strcmp(command, "EXIT") == 0) || (strcmp(command, "quit") == 0))
			{
				printf("Executing built-in exit\n");
				go = false;
			}
			
			else if ((strcmp(command, "which") == 0) || strcmp(command, "WHICH") == 0)
			{
				printf("Executing built-in which\n");
				char *foundCommand = which(args[0], builtIns, args[1], features, pathlist);
				printf("%s\n", foundCommand);
				pathlist = get_path();
			}
			
			else if ((strcmp(command, "where") == 0) || strcmp(command, "WHERE") == 0)
			{
				printf("Executing built-in where\n");
				where(args[0], pathlist, builtIns, features);
				pathlist = get_path();
			}
			
			else if (strcmp(command, "cd") == 0)
			{
				printf("Executing built-in cd\n");
				dirMem = cd(args, pwd, owd, homedir, dirMem, q);
				prev = realloc(prev, (size_t) sizeof(char)*(strlen(dirMem[0]) + 1));
				owd = realloc(owd, (size_t) sizeof(char)*(strlen(dirMem[1]) + 1));
				strcpy(prev, dirMem[0]);
				strcpy(owd, dirMem[1]);
			}
		
			else if ((strcmp(command, "pwd") == 0) || (strcmp(command, "PWD") == 0))
			{
				printf("Executing built-in pwd\n");
				printf("%s\n", owd);
			}
			
			else if ((strcmp(command, "prev") == 0) || (strcmp(command, "previous") == 0))
			{
				printf("Executing built-in prev\n");
				printf("%s\n", prev);
			}

			else if ((strcmp(command, "prompt") == 0) || (strcmp(command, "PROMPT") == 0))
			{
				printf("Executing built-in prompt (debug)\n");
				prompt = prompter(args, prompt, q);
			}
			
			else if ((strcmp(command, "pid") == 0) || (strcmp(command, "PID") == 0))
			{
				printf("Executing built-in pid\n");
				printf("pid = %jd\n", (intmax_t) pid);
			}
			
			else if ((strcmp(command, "history") == 0) || (strcmp(command, "HISTORY") == 0) || (strcmp(command, "hist") == 0))
			{
				printf("Executing built-in history\n");
				mem = hist(command, args, mem, memory, mems, q);
			}
			
			else if ((strcmp(command, "list") == 0) || (strcmp(command, "LIST") == 0))
			{
				printf("Executing built-in list\n");
				listHelper(q, owd, args);
			}
			
			else if (strcmp(command, "printenv") == 0)
			{
				int check = 2;
				check = envCheck(env, args);
				if (check > 0) { printf("Executing built-in printenv\n"); }
				pathlist = get_path();
				envprint(env, args, q);
			}
			
			else if (strcmp(command, "setenv") == 0)
			{
				printf("Executing built-in setenv\n");
				pathlist = get_path();
				envSet(args, env, pathlist, q);
				char *tempHome = findName(env, "HOME");
				strcpy(homedir, tempHome);
				homedir = strtok(homedir, "=");
				homedir = strtok(NULL, "=");
			}
			
			else if ((strcmp(command, "alias") == 0) || (strcmp(command, "ALIAS") == 0))
			{
				printf("Executing built-in alias\n");
			}
			
			else if ((strcmp(command, "kill") == 0) || (strcmp(command, "KILL") == 0) || (strcmp(command, "DESTROY") == 0))
			{
				printf("Executing built-in kill\n");
				kill_proc(args, q);
			}
			
			else if (strcmp(command, "\n") == 0)
			{
				
			}		

			else if (strcmp(command, "debug") == 0)
			{
				
			}					
			// END BUILT IN COMMANDS
			else
			{
				if( (command[0] == '/') || ((command[0] == '.') && ((command[1] == '/') ||(command[1] == '.') && (command[2] == '/'))))
				{
					execute(argsEx[0], argsEx, env, pid);
				}
				
				else
				{
					char *newCmd = quickwhich(command, pathlist);
					if (newCmd != NULL)
					{
						//argsEx[0] = realloc(argsEx[0], (size_t) (strlen(newCmd) + 1) * sizeof(char));
						//strcpy(argsEx[0], newCmd);
						*argsEx[0] = *newCmd;
						execute(newCmd, argsEx, env, pid);
					}
					else
					{
						printf("%s: Command not found.\n", commandlineCONST);
					}
				}
			}
		
			if (go) 
			{ 
				q = 0; 
				fprintf(stderr, "\n%s[%s]>", prompt, owd); 
			}
			else { break; }
		}
	}
	plumber(prompt, commandline, buf, owd, pwd, prev, dirMem, args, memory, pathlist, q, mems, commandlineCONST);
	return 0;
} 

char *which(char *command, char **builtins, char *arg, int features, struct pathelement *pathlist)
{
	if (command == NULL) { return "which: too few arguments"; }
	if (arg != NULL) { return "which: too many arguments"; }
	bool found = false;
	int i = 0;
	pathlist = get_path();
	for (i = 0; i < features; i++) { if (strcmp(command, builtins[i]) == 0) { strcat(command, ": shell built-in command."); return command; } }
	while (pathlist) 
	{
		char str[256];
		strcpy(str, pathlist->element);
		strcat(str, "/");
		strcat(str, command);
		if (access(str, F_OK) == 0)
		{
			strcpy(command, str);
			found = true;
			break;
		}
		pathlist = pathlist->next;
	}
	if (found == false) { strcat(command, ": Command not found."); }
	return command;
}

char *quickwhich(char *command, struct pathelement *pathlist)
{
	bool found = false;
	int i = 0;
	pathlist = get_path();
	while (pathlist) 
	{
		char str[256];
		strcpy(str, pathlist->element);
		strcat(str, "/");
		strcat(str, command);
		if (access(str, X_OK) == 0)
		{
			strcpy(command, str);
			found = true;
			break;
		}
		pathlist = pathlist->next;
	}
	if (found == false) { return NULL; }
	return command;
} 

int where(char *command, struct pathelement *pathlist, char **builtins, int features)
{
	if (command == NULL) { printf("where: too few arguments\n"); return -2; }
	bool found = false;
	int i = 0;
	char str[2046];
	pathlist = get_path();
	for (i = 0; i < features; i++) { if (strcmp(command, builtins[i]) == 0) { printf("%s is a shell built-in\n", command); return 0; } }
	while (pathlist) 
	{
		strcpy(str, pathlist->element);
		strcat(str, "/");
		strcat(str, command);
		if (access(str, F_OK) == 0)
		{ found = true; printf("%s\n", str); }
		pathlist = pathlist->next;
	}
	if (found == false) { printf("%s: Command not found.\n", command); return -1; }
	else { return 1; }
} 
void list ( char *dir)
{
	DIR* directory;
	struct dirent* entrypoint;
	directory = opendir(dir);
	if(directory == NULL) { printf("Can't open directory %s\n", dir); }
	else
	{
		while((entrypoint = readdir(directory)) != NULL) { printf("%s\n", entrypoint->d_name); }
		closedir(directory);
	}
} 

int listCheck(char *dir)
{
	DIR* directory;
	struct dirent* entrypoint;
	directory = opendir(dir);
	if(directory == NULL) { return 0; }
	else { return 1; } 
}

void listHelper(int q, char *owd, char **args)
{
	if (q == 1) { printf("%s: \n", owd); list(owd); }
	else
	{
		int i;
		for(i = 0; args[i] != NULL; i++)
		{
			if (listCheck(args[i]) > 0)
			{
				printf("\n");
				printf("%s: \n", args[i]);
				list(args[i]);
			}
			else
			{
				printf("\n");
				printf("Cannot open directory: %s\n", args[i]);
			}
		}
	}
}

char *prompter(char **args, char *prompt, int q)
{
	
	char *buffer = calloc(MAX_CANON, sizeof(char));
	if (q > 2) 
	{
		printf("prompt: too many arguments.\n"); 
		return prompt;
	}
	
	else if (q == 2)
	{
		strcat(buffer, args[0]);
		strcat(buffer, " ");
		return buffer;
	}
	
	else
	{
		printf("input prompt prefix:");
		fgets(buffer, BUFFER, stdin);
		buffer[strlen(buffer) - 1] = 0;
		strcat(buffer, " ");
		return buffer;
	}
}

int hist(char *command, char **args, int mem, char **memory, int mems, int q)
{
	if (q > 2) { printf("history: too many arguments."); return mem; }
	else
	{
		if (args[0] != NULL)
		{
			int i = 0;
			mem = atoi(args[0]);
			if (mem > mems) { mem = mems; }
			for (i = 0; i < mem; i++) { printf("(%d): %s\n", i, memory[i]); }
			return mem;
		}
		else 
		{
			int i = 0;
			if (mem > 10) { mem = 10; }
			for (i = 0; i < mem; i++) { printf("(%d): %s\n", i, memory[i]); }
			return mem;
		}
	}
}

int envCheck(char **env, char **args)
{
	int e = 0;
	char *temp;
	char temp2[2046] = "init";
	bool found = false;
	if (args[1] != NULL) { return 0; }
	else
	{
		if (args[0] == NULL) { return 1; }
		else 
		{
			for (e = 0; env[e] != NULL; e++)	
			{    
				strcpy(temp2, env[e]);
				temp = strtok(temp2, "=");
				if (strcmp(args[0], temp) == 0)
				{ found = true; }
			}
			if (found == false) { return 0; }
			else { return 1; }
		}
	}
}

void envprint(char **env, char **args, int q)
{
	int e = 0;
	char *temp;
	char temp2[1024] = "init";
	if (q > 2) { printf("printenv: Too many arguments.\n"); }
	else
	{
		if (args[0] == NULL) { for (e = 0; env[e] != NULL; e++) { printf("%s\n", env[e]); } }
		else 
		{
			for (e = 0; env[e] != NULL; e++)	
			{    
				strcpy(temp2, env[e]);
				temp = strtok(temp2, "=");
				if (strcmp(args[0], temp) == 0) { char *variable = findName(env, args[0]); printf("%s\n", variable); }
			}
		}
	}
}

void envSet(char **args, char **env, struct pathelement *pathlist, int q)
{
	bool new = false;
	char *variable = findName(env, args[0]);
	int variables = countEntries(env);
	if (variable == NULL) { new = true; }
	
	if (q == 1)
	{
		envprint(env, args, q);
	}
	
	else if (q == 2)
	{
		if (new)
		{
			env = reinitEnv(env);
			newEnvVar(env, args[0], " ");
		}
		else { printf("Improper usage of setenv.\n"); }
		if (strcmp(args[0], "PATH") == 0) { pathPlumber(pathlist); pathlist = get_path(); }
	}
	
	else if (q == 3)
	{
		env = reinitEnv(env);
		newEnvVar(env, args[0], args[1]);
		if (strcmp(args[0], "PATH") == 0)
		{
			pathPlumber(pathlist);
			pathlist = get_path();
		}
	}
	
	else
	{
		printf("setenv: Too many arguments.\n");
	}
}

char **reinitEnv(char **env)
{
	int variables = countEntries(env);
	char **newEnv = calloc(variables * 2, sizeof(char*));
	for (int i = 0; env[i] != NULL; i++)
	{
		char *oldVar = malloc(strlen(env[i]) + 1);
		sprintf(oldVar, "%s", env[i]);
		newEnv[i] = oldVar;
	}
	return newEnv;
}


void newEnvVar(char **env, char *name, char *value)
{
	char *newVar = malloc(strlen(name) + strlen(value) + 2);
	sprintf(newVar, "%s=%s", name, value);
    putenv(newVar);
}

char *findName(char **envi, char *name)
{
	if (name == NULL) { return NULL;}
	int i = 0;
	for (i = 0; envi[i] != NULL; i++)
	{
		char *temp = malloc(strlen(envi[i]) + 1);
		char *temp2 = malloc(strlen(temp) + 1);
		strcpy(temp, envi[i]);
		temp2 = strtok(temp, "=");
		if (strcmp(name, temp2) == 0) { return envi[i];}
	}
	return NULL;
}


int countEntries(char **array)
{
	int i = 0;
	int count = 0;
	for (i = 0; array[i] != NULL; i++) { count++; }
	return count;
}

char **cd (char **args, char *pwd, char *owd, char *homedir, char **dirMem, int q)
{
	char *prev = calloc(strlen(dirMem[0]) + 1, sizeof(char));
	if (q > 2) 
	{
		printf("cd: Too many arguments.\n");
		free(prev);
		return dirMem;
	}
	
	else
	{
		if (args[0] == NULL) 
		{
			free(dirMem[0]);
			dirMem[0] = calloc(strlen(owd) + 1, sizeof(char));
			strcpy(dirMem[0], owd);
			if (chdir(homedir) != 0) { perror("cd"); }
		} 
		
		else if (strcmp(args[0], "-") == 0)
		{
			
			strcpy(prev, dirMem[0]);
			if (chdir(prev) != 0) { perror("cd"); }
			else
			{
				strcpy(prev, dirMem[0]);
				strcpy(dirMem[0], owd);
			}
		}
		
		else 
		{
			if (chdir(args[0]) != 0) { perror("cd"); }
			else
			{
				free(dirMem[0]);
				dirMem[0] = calloc(strlen(owd) + 1, sizeof(char));
				strcpy(dirMem[0], owd);
			}
		}
		
		if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL ) { perror("getcwd"); exit(2); }
		free(dirMem[1]);
		dirMem[1] = calloc(strlen(pwd) + 1, sizeof(char));
		strcpy(dirMem[1], pwd);
		free(prev);
		return dirMem;
	}
}

char *get_pwd() 
{
  char cwd[4096];
  if (getcwd(cwd,sizeof(cwd))!=NULL) { return cwd; }
  else { perror("get_pwd"); }
  return NULL;
}

void plumber(char *prompt, char *commandline, char *buf, char *owd, char *pwd, char *prev, char **dirMem, char **args, char **memory, 
struct pathelement *pathlist, int q, int mems, char *commandlineCONST)
{
	free(prompt);		free(commandline);	free(buf);
	free(owd);			free(pwd);			free(prev);
	free(commandlineCONST);
	free(dirMem[0]); 	free(dirMem[1]);	free(dirMem);
	for (int i = 0; i < q; i++) { free(args[i]); } free(args);
	for (int i = 0; i < mems; i++) { free(memory[i]); } free(memory); 
	pathPlumber(pathlist);
}


void pathPlumber(struct pathelement *head)
{
  struct pathelement *current = head;
  struct pathelement *temp;
  while(current->next != NULL)
  {
    temp = current;
    current = current->next;
    free(temp);
  }
  free(current);
}

/* Signal Handler for SIGINT */
void sigintHandler(int sig_num){
  signal(SIGCHLD, sigintHandler);
  fflush(stdout);
  return;
}

void signalSTPHandler(int sig_num){
  signal(SIGTSTP, signalSTPHandler);
  //printf("Can't terminate process with Ctrl+Z %d \n", waitpid(getpid(),NULL,0));
  fflush(stdout);
  return;
}

int execute(char *cmd, char **argv, char **env, pid_t pid)
{

	pid = fork();
    int child_status;

    if(pid == 0) //** Executed in child process
	{                        

        execve(cmd, argv, env);
        
        // Exec commands only return if there's an error
        perror("Error in execute()");        
        
        // We exit since the process image will be replaced with itself here and
        // we will need to enter "exit" twice to truely exit.
        exit(0);        

    } 
	else if(pid > 0) //** Executed in parent process
	{                
            // TODO: add signal handler for SIGCHLD
            if(waitpid(pid, &child_status, 0) == -1)
			{
                perror("Error in waitpid");
            }
			if(WEXITSTATUS(child_status) != 0)
			{
				printf("Exited with code: %d\n", WEXITSTATUS(child_status)); // Print out the exit status if it is non-zero
			}
    } 
	else  //** Didn't fork properly 
	{
        perror("Fork failed\n");
    }

    return child_status;
}

int lineHandler(int *q, char ***args, char ***argv, char *commandline)
{
	int len = strlen(commandline);

	//## Tokenize the command into the argv array
	char* ptr = NULL;
	char* token = strtok_r(commandline, " ", &ptr);


	// If the line is blank, the first token will be the null string.
	if(token == '\0') { return 0; }       

	*args[0] = token;        // argv[0] is the command name

	for(int i = 1; token != NULL && i < MAXTOK; i++)
	{
		token = strtok_r(NULL, " \t", &ptr);
		(*args)[i] = token;
		(*argv)[i-1] = token;
		*q = i;
	}

	return 1;
}

void kill_proc(char **args, int q)
{
	if (q == 1) 
	{
		printf("Improper usage of kill.\n");
	}
	
	else if (q == 2)
	{
		kill(atoi(args[0]), SIGTERM);
	}
	else if(strstr(args[0], "-") != NULL)
	{
		kill(atoi(args[1]), atoi(++args[0]));
	}
}