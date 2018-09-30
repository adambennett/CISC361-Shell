#include "sh.h"

int main( int argc, char **argv, char **envp )
{
	/* put signal set up stuff here */
	signal (SIGTSTP, sig_handler);

	return sh(argc, argv, envp);
}

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

void sig_handler(int signal)
{
	if(signal == SIGTSTP)
	{
		fflush(stdout);
	}
}



