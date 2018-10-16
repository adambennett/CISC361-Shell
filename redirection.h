#ifndef redir_h
#define redir_h

#include "sh.h"

int parse_redirection(char** command, char** file, char* line);
void perform_redirection(int* fid, char* redirect_file, int rt);
void reset_redirection(int* fid, int redirection_type);
int proc_redirect(char **line_in, int fid);

#endif //redir_h