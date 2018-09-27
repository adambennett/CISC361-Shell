#ifndef signal_h
#define signal_h

#include "sh.h"

void sigintHandler(int sig_num);
void signalSTPHandler(int sig_num);

#endif //signal_h