#ifndef main_h
#define main_h

#include "sh.h"

void sigintHandler(int sig_num);
void signalSTPHandler(int sig_num);
void sig_handler(int signal); 

#endif //main_h