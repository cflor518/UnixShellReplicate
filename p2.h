#include <unistd.h> //access(),execvp, dup2,chdir, _exit, fork(),setpgid(),getpgrp(), link(),unlink()
#include <stdio.h> //perror,fflush()
#include <errno.h> //perror
#include <stdlib.h> //getenv()
#include <fcntl.h> //For O_APPEND and O+WRONLY,link 
#include <signal.h> //signal(),killpg()
#include <sys/types.h> //open(), fork(),opendir(), wait(),setpgid(),getpggrp()
#include <dirent.h> //opendir()
#include <sys/wait.h> //wait()
#include <sys/stat.h> //open,creat
#include <fcntl.h> //open
#include "getword.h"
#define MAXITEM 100 /* max number of words per line */
#define NEW_LINE_SIGNAL -10
#define EOF_SIGNAL 0
#define DELIM_SIGNAL -1
#define MAXARGS 30 
//The following two are so that my code reads
//more natually in p2.c
#define MATCHING_STRING 0 
#define NULL_CHAR 1
#define PIPE_AMT_LIMIT 10
#define PIPE_ENDS 2
#define PIPE_IN 2
#define PIPE_OUT 1
#define NEXT_PIPE 2
#define OFFSET 1
#define REGULAR_PIPE -1
#define SPECIAL_PIPE 1
#define HAVENT_FOUND_IT 0
#define NO_PROBLEM 0
#define NOT_FOUND -1

//The parser use the program getword which is a lexical analyzer. The parser
//is responsible of putting the token recieved by the lexical analyzer and putting
//them in a buffer, based on the token there is another array called newargv which
//point to vital information that is to be executed. This function takes care of 
//what ends up in the buffer, what things are pointed to by the buffer and what 
//things are ignored.
int parser();
//In the manner which i filled my buffer, I had newargv point to the filenames inside 
//the buffer. Because of this I needed the execvp command not to "see" certain things
//that newargv pointed at because some things arent supposed to be executed by execvp.
//The side effect that I noticed when coding this way is that commands like
//echo < a cesar flores, wont work because the command echo sees null before it sees cesar
//flores. 

void forkPlummerChildren(int);
