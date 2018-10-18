#include "sh.h"

const int NUM_REDIRECT_OPERATORS = 5;
const char* REDIRECT_OPERATORS[] = { ">>&", ">>", ">&", ">", "<" };    


/** 
 * @brief Sets redirection type and filename from commandline
 *
 * Checks the commandline for redirection operators, and creates an integer
 * representation of the found operator if one exists. The integer is a code
 * for which operator is found, for example if 1 is returned, the operator
 * is '>>', which represents read standard out and append (RD_STDOUT_APPEND).
 * This also determines the filename we may wish to redirect into, again by
 * searching over the commandline.
 * 			
 * @param command  			The new commandline we (attempt to) fill with the proper command to execute after redirection is complete
 * @param file				The string we fill with the file name, for further use in proc_redirect
 * @param line				The commandline we pass in
 *
 * NOTE: Return value info below
 * 	0 - >>& - RD_ALL_APPEND
 *	1 - >> - RD_STDOUT_APPEND
 *	2 - >& - RD_ALL
 *	3 - > - RD_STDOUT
 *	4 - < - RD_STDIN
 *	-1 - RD_NONE
 *
 * @return Returns integer value associated with the type of redirection the user has indicated.
 */
int parse_redirection(char** command, char** file, char* line)
{
    char* rd_stdout = NULL;
	int redirect_code;
    for(int i=0; i < NUM_REDIRECT_OPERATORS && rd_stdout == NULL; i++) { rd_stdout = strstr(line, REDIRECT_OPERATORS[i]); redirect_code = i; }
    if(rd_stdout == NULL) { return -1; }
	int command_length = strlen(rd_stdout) - strlen(line);
	if (command_length < 0) { command_length = command_length * -1; }
    *command = (char*)malloc(strlen(line) + 1);
    memcpy(*command, line, strlen(line) + 1);
    (*command)[strlen(*command) - 1] = '\0';
    char* ptr = strtok(line + command_length, " >&<");
    *file = (char*)malloc(strlen(ptr) + 1); 
    memcpy(*file, ptr, strlen(ptr) + 1);
    return redirect_code;
}

/** 
 * @brief Performed the redirection actions.
 * 
 * @param fid 				The file id of the redirect file.
 * @param redirect_file 	The path to the redirect file.
 * @param rt 				The redirecttion type from ::redirect_opcodes.
 */
void perform_redirection(int* fid, char* redirect_file, int rt)
{
    int open_flags = O_CREAT;

    // Assign read/write mode
    switch(rt)
	{
        case 0:
        case 1:
        case 2:
        case 3:
            open_flags |= O_WRONLY;
            break;
        case 4:
            open_flags |= O_RDONLY; 
            break;
    }
	
	// 0 - >>& - RD_ALL_APPEND
	// 1 - >> - RD_STDOUT_APPEND
	// 2 - >& - RD_ALL
	// 3 - > - RD_STDOUT
	// 4 - < - RD_STDIN
	// -1 - RD_NONE

    // Assign append mode
    switch(rt)
	{
        case 0:
        case 1:
            open_flags |= O_APPEND;
            break;
        case 2:
        case 3:
        case 4:
            break;
        default:
            break;
    }

    *fid = open(redirect_file, open_flags, 0666);

    // Perform the redirection
    switch(rt)
	{
        case 0:
        case 2:
            close(2);
            dup(*fid);
            // Fall through (we never redirect only stderr)
        case 1:
        case 3:
            close(1);
            dup(*fid);
            close(*fid);
            break;
        case 4:
            close(0);
            dup(*fid);
            close(*fid);
            break;
        default:
            break;
    }

}

/** 
 * @brief Resets redirection so that stdin, stdout, and stderr all go to the terminal.
 * 
 * @param fid 				Redirection file.
 * @param redirection_type 	Redirection type from ::redirection_opcodes.
 */
void reset_redirection(int* fid, int redirection_type)
{
    if(redirection_type != -1 && redirection_type != 4)
	{
        *fid = open("/dev/tty", O_WRONLY);
        close(2);
        dup(*fid);
        close(*fid);

        *fid = open("/dev/tty", O_WRONLY);
        close(1);
        dup(*fid);
        close(*fid);
    } 
	
	else if(redirection_type == 4)
	{
        *fid = open("/dev/tty", O_RDONLY);
        close(0);
        dup(*fid);
        close(*fid);
    }
}

/** 
 * @brief Called from during user input loop to handle redirection
 *
 * Uses the above functions in tandem to handle redirection properly
 * and simply from sh.c
 * 
 * @param line_in 			The commandline we pass in from the user input loop
 * @param fid 				The file id of the redirect file.
 */
int proc_redirect(char **line_in, int fid)
{
	//## Process redirection operators
	char* command_line = NULL;
	char* redirect_file = NULL;
	char *tempLine = *line_in;
	int redirection_type = parse_redirection(&command_line, &redirect_file, tempLine);

	if(redirection_type >= 0)
	{
		// If noclobber is 0
		if ((strcmp(getenv("NOCLOB"), "Not set") == 0))
		{
			// > and >&
			if (redirection_type == 2 || redirection_type == 3)
			{
				if ((access(redirect_file, F_OK) != -1)) {if(remove(redirect_file) == -1) { perror("Error removing existing redirect file"); } }
			}
			perform_redirection(&fid, redirect_file, redirection_type);
			return redirection_type;
		}
		// If noclobber is 1
		else
		{
			// > and >&
			if (redirection_type == 2 || redirection_type == 3)
			{
				// File does not exist so we want to create it and redirect
				if ((access(redirect_file, F_OK) == -1))
				{
					perform_redirection(&fid, redirect_file, redirection_type);
					return redirection_type;
				}
				
				// File exists so we just print error and return
				else
				{
					printf("%s: File exists.\n", redirect_file);
					return -3;
				}
			}
			
			// >> and >>&
			else if (redirection_type == 0 || redirection_type == 1)
			{
				// File does not exist we just print error and return
				if ((access(redirect_file, F_OK) == -1))
				{
					printf("%s: File does not exist.\n", redirect_file);
					return -3;
				}
				
				// File exists so we want to redirect and append
				else
				{
					perform_redirection(&fid, redirect_file, redirection_type);
					return redirection_type;
				}
			}
			
			// <
			else
			{
				perform_redirection(&fid, redirect_file, redirection_type);
				return redirection_type;
			}
		}
	}
	
	return redirection_type;
}