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
	//char **argsEx = calloc(MAXARGS, sizeof(char*));
	char **memory = calloc(MAXMEM, sizeof(char*));
	char **dirMem = calloc(MAXARGS, sizeof(char*));
	char **env = envp;
	//char ***test1 = &env;
	//char ***test2 = &envp;
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
		while ((fgets(commandline, BUFFER, stdin) != NULL) && go) 
		{
			
			if (commandline[strlen(commandline) - 1] == '\n')
			{
				if (strlen(commandline) > 1) 
				{
					commandline[strlen(commandline) - 1] = 0; 	// replace newline with null
				} 
				
				strcpy(commandlineCONST, commandline);
				memory[h] = calloc(strlen(commandline) + 1, sizeof(char));
				strcpy(memory[h], commandline);
				h++; mems++;
				if (mem < 10) { mem++; }
				if (mem > 10) { mem = 10; }
				command = strtok(commandline, " ");
				args[0] = strtok(NULL, " ");
				while (args[q] != NULL)
				{
					q++;
					args[q] = strtok(NULL, " ");
					args[q+1] = NULL;
				}				
			}
			
			// BUILT IN COMMANDS //
			if ((strcmp(command, "exit") == 0) || (strcmp(command, "EXIT") == 0) || (strcmp(command, "quit") == 0))
			{
				printf("Executing built-in exit\n");
				go = false;
				plumber(prompt, commandline, buf, owd, pwd, prev, dirMem, args, memory, pathlist, q, mems, commandlineCONST);
				return 0;
			}
			
			else if ((strcmp(command, "which") == 0) || strcmp(command, "WHICH") == 0)
			{
				printf("Executing built-in which\n");
				char *foundCommand;
				foundCommand = calloc(100, sizeof(char));
				foundCommand = which(args[0], builtIns, args[1], features);
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
				dirMem = cd(args, pwd, owd, homedir, dirMem);
				//free(pwd);
				//free(owd);
				//prev = calloc(strlen(dirMem[0]) + 1, sizeof(char));
				//owd = calloc(strlen(dirMem[1]) + 1, sizeof(char));
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

			else if ((strcmp(command, "prompt") == 0) || (strcmp(command, "PROMPT") == 0))
			{
				printf("Executing built-in prompt\n");
				char *temp = prompter(args, prompt, buf);
				free(prompt);
				prompt = calloc(strlen(temp) + 1, sizeof(char));
				strcpy(prompt, temp);
			}
			
			else if ((strcmp(command, "pid") == 0) || (strcmp(command, "PID") == 0))
			{
				printf("Executing built-in pid\n");
				printf("pid = %jd\n", (intmax_t) pid);
			}
			
			else if ((strcmp(command, "history") == 0) || (strcmp(command, "HISTORY") == 0) || (strcmp(command, "hist") == 0))
			{
				printf("Executing built-in history\n");
				mem = hist(command, args, mem, memory, mems);
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
				envprint(env, args);
			}
			
			else if (strcmp(command, "setenv") == 0)
			{
				printf("Executing built-in setenv\n");
				pathlist = get_path();
				char **envTemp = env;
				int environs = countEntries(envp);
				char **newEnv = malloc(sizeof(env) * 2);
				newEnv = envSet(args, env, pathlist, q);
				env = newEnv;
				char *tempHome;
				tempHome = calloc(strlen(getenv("HOME")) + 1, sizeof(char));
				memcpy(tempHome, getenv("HOME"), sizeof(char*));
				strcpy(homedir, tempHome);
				free(tempHome);
				
			}
			
			else if ((strcmp(command, "alias") == 0) || (strcmp(command, "ALIAS") == 0))
			{
				printf("Executing built-in alias\n");
			}
			
			else if ((strcmp(command, "kill") == 0) || (strcmp(command, "KILL") == 0) || (strcmp(command, "DESTROY") == 0))
			{
				printf("Executing built-in kill\n");
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
				if ((pid = fork()) < 0) 
				{
					//err_sys("fork error");
				} 
				else if (pid == 0) 
				{		/* child */
					execlp(commandlineCONST, commandlineCONST, (char *)0);
					//execve(commandline, args, env);
					printf("%s: Command not found.\n", command);
					//err_ret("couldn't execute: %s", buf);
					exit(127);
				}

				/* parent */
				if ((pid = waitpid(pid, &status, 0)) < 0)
					//err_sys("waitpid error");
				printf("%% ");
			}
		
			if (go) 
			{ 
				fprintf(stderr, "\n%s[%s]>", prompt, owd); 
				for (int i = 0; i < q; i++)
				{
					args[i] = NULL;
				}
				q = 0; 
			}
			else { break; }
		}
	}
	plumber(prompt, commandline, buf, owd, pwd, prev, dirMem, args, memory, pathlist, q, mems, commandlineCONST);
	return 0;
} /* sh() */

char *which(char *command, char **builtins, char *arg, int features)
{
	/* loop through pathlist until finding command and return it.  Return
	NULL when not found. */
	if (command == NULL) { return "which: too few arguments"; }
	if (arg != NULL) { return "which: too many arguments"; }
	
	bool found = false;
	int i = 0;
	char str[256];
	struct pathelement *pathlist = get_path();
	
	for (i = 0; i < features; i++)
	{
		if (strcmp(command, builtins[i]) == 0)
		{
			strcat(command, ": shell built-in command.");
			return command;
		}
	}
	
	while (pathlist) 
	{
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
	//pathPlumber(pathlist);
	return command;
} /* which() */

char *quickwhich(char *command, struct pathelement *pathlist, char *arg, int q, char **args)
{
	/* loop through pathlist until finding command and return it.  Return
	NULL when not found. */
	
	bool found = false;
	int i = 0;
	char str[256];
	pathlist = get_path();
	
	
	while (pathlist) 
	{
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
	
	if (found == false) { pathPlumber(pathlist); return NULL; }
	//else { for (i = 0; i < q; i++) { strcat(command, " "); strcat(command, args[i]); strcat(command, " "); } }
	pathPlumber(pathlist);
	return command;
} /* which() */

int where(char *command, struct pathelement *pathlist, char **builtins, int features)
{
	/* similarly loop through finding all locations of command */
	if (command == NULL) 
	{ 
		printf("where: too few arguments\n"); 
		return -2;
	}
	
	bool found = false;
	int i = 0;
	char str[2046];
	pathlist = get_path();
	
	for (i = 0; i < features; i++)
	{
		if (strcmp(command, builtins[i]) == 0)
		{
			printf("%s is a shell built-in\n", command);
			return 0;
		}
	}
	
	while (pathlist) 
	{
		strcpy(str, pathlist->element);
		strcat(str, "/");
		strcat(str, command);
		if (access(str, F_OK) == 0)
		{
			found = true;
			printf("%s\n", str);
		}
		pathlist = pathlist->next;
	}
	
	if (found == false) 
	{ 
		printf("%s: Command not found.\n", command); 
		return -1;
	}
	
	else { return 1; }
} /* where() */

void list ( char *dir)
{
	DIR* directory;
	struct dirent* entrypoint;
	directory = opendir(dir);
	if(directory == NULL)
	{
		printf("Can't open directory %s\n", dir);
	}
	else
	{
		while((entrypoint = readdir(directory)) != NULL)
		{
			printf("%s\n", entrypoint->d_name);
		}
	closedir(directory);
	}


  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
} /* list() */

int listCheck(char *dir)
{
	DIR* directory;
	struct dirent* entrypoint;
	directory = opendir(dir);
	
	if(directory == NULL)
	{
		//printf("Can't open directory %s\n", dir);
		return 0;
	}
	else
	{
		while((entrypoint = readdir(directory)) != NULL)
		{
			//printf("%s\n", entrypoint->d_name);
		}
	closedir(directory);
	return 1;
	}
}

void listHelper(int q, char *owd, char **args)
{
	if (q == 0)
	{
		printf("%s: \n", owd);
		list(owd);
	}
	else
	{
		int i;
		for(i = 0; i < q; i++)
		{
			if (listCheck(args[i]) > 0)
			{
				printf("\n");
				printf("%s: \n", args[i]);
				list(args[i]);
			}
		}
	}
}

char *prompter(char **args, char *prompt, char *commandline)
{
	if (args[1] != NULL) 
	{ 
		printf("prompt: too many arguments.\n"); 
		return prompt;
	}
	else 
	{
		if (args[0] == NULL)
		{
			printf("input prompt prefix:");
			fgets(commandline, BUFFER, stdin);
			commandline[strlen(commandline) - 1] = 0;
			strcat(commandline, " ");
			return commandline;
		}
		else
		{
			strcat(args[0], " ");
			return args[0];
		}
	}
}

int hist(char *command, char **args, int mem, char **memory, int mems)
{
	if (args[1] != NULL)
	{
		printf("history: too many arguments.");
		return mem;
	}
	
	else
	{
		if (args[0] != NULL)
		{
			int i = 0;
			mem = atoi(args[0]);
			if (mem > mems) { mem = mems; }
			//if (!isdigit(mem)) { printf("history: Badly formed number.\n"); return mems; }
			for (i = 0; i < mem; i++)
			{
				printf("(%d): %s\n", i, memory[i]);
			}
			return mem;
		}
		else 
		{
			int i = 0;
			if (mem > 10) { mem = 10; }
			for (i = 0; i < mem; i++)
			{
				printf("(%d): %s\n", i, memory[i]);
			}
			return mem;
		}
	}
	
	
}

int envCheck(char **env, char **args)
{
	int e = 0;
	char *temp;
	char temp2[128] = "init";
	bool found = false;
	char **envi;
	envi = env;
	if (args[1] != NULL) 
	{
		return 0; 
	}
	else
	{
		if (args[0] == NULL)
		{
			return 1;
		}
		else 
		{
			for (e = 0; envi[e] != NULL; e++)	
			{    
				strcpy(temp2, envi[e]);
				temp = strtok(temp2, "=");
				if (strcmp(args[0], temp) == 0)
				{
					found = true;
				}
			}
			if (found == false) { return 0; }
			else { return 1; }
		}
	}
}

void envprint(char **env, char **args)
{
	int e = 0;
	char *temp;
	char temp2[128] = "init";
	char **envi;
	envi = env;
	if (args[1] != NULL) { printf("printenv: Too many arguments.\n"); }
	else
	{
		if (args[0] == NULL)
		{
			for (e = 0; envi[e] != NULL; e++)	
			{    
				printf("%s\n", envi[e]);
			}
			
		}
		else 
		{
			for (e = 0; envi[e] != NULL; e++)	
			{    
				strcpy(temp2, envi[e]);
				temp = strtok(temp2, "=");
				if (strcmp(args[0], temp) == 0)
				{
					printf("%s\n", getenv(args[0]));
				}
			}
			
		}
	}
}

/*
void set_env(char *name, char *value, char **env)
{
	char **envi;
	envi = env;
	bool new = false;
	char *variable;
	variable = findName(envi, name);
	int variables = countEntries(envi);
	if (variable == NULL) { new = true; }
	
	char *temp = calloc(100, sizeof(char));
	strcat(temp, name);
	strcat(temp, "=");
	strcat(temp, value);
	envi = malloc(sizeof(envi) * 2);
	for (int r = 0; r < variables; r++)
	{
		envi[r] = malloc(sizeof(char*));
		envi[r] = env[r];
	}
	envi[variables + 1] = malloc(sizeof(char*));
	envi[variables + 2] = malloc(sizeof(char*));
	strcpy(envi[variables + 1], temp);
	envi[variables + 2] = NULL;
	setenv(name, value, 1);
	free(temp);
	//for (r = 0; r < variables + 2; r++)
	//{
	//	free(envi[r]);
	//}
	//free(envi);
	
	
}
*/

char **envSet(char **args, char **env, struct pathelement *pathlist, int argCount)
{
	
	char **envi;
	envi = env;
	int q = argCount;
	bool new = false;
	char *variable;
	variable = findName(envi, args[0]);
	int variables = countEntries(envi);
	if (variable == NULL) { new = true; }
	
	if (q == 0)
	{
		envprint(envi, args);
		return envi;
	}
	
	else if (q == 1)
	{
		if (new) 
		{ 
			char *temp = calloc(100, sizeof(char));
			strcat(temp, args[0]);
			strcat(temp, "=");
			strcat(temp, "default");
			envi = malloc(sizeof(envi) * 2);
			for (int r = 0; r < variables; r++)
			{
				envi[r] = malloc(sizeof(char*));
				envi[r] = env[r];
			}
			envi[variables + 1] = malloc(sizeof(char*));
			envi[variables + 2] = malloc(sizeof(char*));
			strcpy(envi[variables + 1], temp);
			envi[variables + 2] = NULL;
			setenv(args[0], " ", 1);
			free(temp);
		}
		else { printf("Improper usage of setenv.\n"); }
		if (strcmp(args[0], "PATH") == 0) { pathPlumber(pathlist); pathlist = get_path(); }
		return envi;
	}
	
	else if (q == 2)
	{
		if (!new)
		{
			if (strlen(variable) < strlen(args[1]))
			{
				char *temp = calloc(100, sizeof(char));
				strcat(temp, args[0]);
				strcat(temp, "=");
				strcat(temp, args[1]);
				envi = malloc(sizeof(envi) * 2);
				for (int r = 0; r < variables; r++)
				{
					envi[r] = malloc(sizeof(char*));
					envi[r] = env[r];
				}
				envi[variables + 1] = malloc(sizeof(char*));
				envi[variables + 2] = malloc(sizeof(char*));
				strcpy(envi[variables + 1], temp);
				envi[variables + 2] = NULL;
				setenv(args[0], args[1], 1);
				free(temp);
				//printf("Can you not please? That string is too long and C is hard ok?\n");
				
			}
			else { setenv(args[0], args[1], 1); }
		}
		else
		{
			char *temp = calloc(100, sizeof(char));
			strcat(temp, args[0]);
			strcat(temp, "=");
			strcat(temp, args[1]);
			envi = malloc(sizeof(envi) * 2);
			for (int r = 0; r < variables; r++)
			{
				envi[r] = malloc(sizeof(char*));
				envi[r] = env[r];
			}
			envi[variables + 1] = malloc(sizeof(char*));
			envi[variables + 2] = malloc(sizeof(char*));
			strcpy(envi[variables + 1], temp);
			envi[variables + 2] = NULL;
			setenv(args[0], args[1], 1);
			free(temp);
		}
		if (strcmp(args[0], "PATH") == 0)
		{
			pathPlumber(pathlist);
			pathlist = get_path();
		}
		
		return envi;
	}
	
	else
	{
		printf("setenv: Too many arguments.\n");
		return envi;
	}

}

char *findName(char **envi, char *name)
{
	if (name == NULL) { return NULL;}
	int i = 0;
	char *temp = malloc(sizeof(char*));
	char *temp2 = malloc(sizeof(char*));
	for (i = 0; envi[i] != NULL; i++)
	{
		//strcpy(temp, envi[i]);																			// VALGRIND
		memcpy(temp, envi[i], sizeof(envi[i]));
		//temp2 = strtok(temp, "=");
		memcpy(temp2, strtok(temp, "="), sizeof(temp));
		if (strcmp(name, temp2) == 0)
		{
			free(temp); free(temp2);
			return envi[i];
		}
	}
	
	free(temp);
	free(temp2);
	return NULL;
}

/*
int findPosition(char **envi, char *name)
{
	if (name == NULL) { return -1; }
	int i = 0;
	char *temp = malloc(sizeof(char*));
	char *temp2 = malloc(sizeof(char*));
	for (i = 0; envi[i] != NULL; i++)
	{
		//strcpy(temp, envi[i]);																			// VALGRIND
		memcpy(temp, envi[i], sizeof(envi[i]));
		//temp2 = strtok(temp, "=");
		memcpy(temp2, strtok(temp, "="), sizeof(temp));
		if (strcmp(name, temp2) == 0)
		{
			free(temp); free(temp2);
			return i;
		}
	}
	
	free(temp);
	free(temp2);
	return -1;
}
*/

int countEntries(char **array)
{
	int i = 0;
	int count = 0;
	for (i = 0; array[i] != NULL; i++)
	{
		count++;
	}
	
	return count;
}

char **cd (char **args, char *pwd, char *owd, char *homedir, char **dirMem)
{
	char *prev = calloc(strlen(dirMem[0]) + 1, sizeof(char));
	if (args[1] != NULL) 
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
			if (chdir(homedir) != 0) 
			{
				perror("cd");
			}
		} 
		
		else if (strcmp(args[0], "-") == 0)
		{
			
			strcpy(prev, dirMem[0]);
			if (chdir(prev) != 0) 
			{
				perror("cd");
			}
			else
			{
				strcpy(prev, dirMem[0]);
				strcpy(dirMem[0], owd);
			}
		}
		
		else 
		{
			//printf("args[0]: %s\n", args[0]);
			if (chdir(args[0]) != 0) 
			{
				perror("cd");
			}
			else
			{
				free(dirMem[0]);
				dirMem[0] = calloc(strlen(owd) + 1, sizeof(char));
				strcpy(dirMem[0], owd);
			}
		}
		
		if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
		{
			perror("getcwd");
			exit(2);
		}
		
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
  if (getcwd(cwd,sizeof(cwd))!=NULL) {
    return cwd;
  }
  else {
    perror("pwd");
  }
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
	//for (int i = 0; i < q + 1; i++) { free(argsEx[i]); } free(argsEx);
	for (int i = 0; i < mems; i++) { free(memory[i]); } free(memory); 
	//while(pathlist) { struct pathelement *n1 = pathlist; pathlist = pathlist->next; free(n1); }
	pathPlumber(pathlist);
}


void pathPlumber(struct pathelement *head){
  struct pathelement *current = head;
  struct pathelement *temp;
  while(current->next != NULL){
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
