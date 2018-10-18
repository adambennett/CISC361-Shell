#ifndef redir_h
#define redir_h

#include "sh.h"

int parse_redirection(char** command, char** file, char* line);				//Sets redirection type and filename from commandline
void perform_redirection(int* fid, char* redirect_file, int rt);			//Performed the redirection actions.
void reset_redirection(int* fid, int redirection_type);						//Resets redirection so that stdin, stdout, and stderr all go to the terminal
int proc_redirect(char **line_in, int fid);									//Called from during user input loop to handle redirection

#endif //redir_h