#include "sh.h"

int main( int argc, char **argv, char **envp )
{
	signal (SIGTSTP, sig_handler);
	return sh(argc, argv, envp);
}

void sigintHandler(int sig_num) { signal(SIGCHLD, sigintHandler); fflush(stdout); return; }
void signalSTPHandler(int sig_num) { signal(SIGTSTP, signalSTPHandler); fflush(stdout); return; }
void sig_handler(int signal) { if(signal == SIGTSTP) { fflush(stdout); } }



