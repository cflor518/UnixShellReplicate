/********************************************************
 * Program2 
 * Programmer: Cesar Flores
 * Professor: John Carroll
 * Class: CS570
 * Date Started: 1/26/18
 * Last Updated: 2/9/2018
 * Due Date: 2/28/2018 at 8pm.
 *
 * Program 2 : This program mimics a shell with very limited
 * features compared to that of a real shell. This program
 * can execute files and can execute any valid system call
 * handed to it. It can handle single pipes and will respind
 * to the ampersand command (don't wait). There are some
 * anamolies, please see gradernotes file.
 * ******************************************************/
#include "getword.h"
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
#include "CHK.h"
#define NEW_LINE_SIGNAL -10
#define EOF_SIGNAL 0
#define DELIM_SIGNAL -1
#define NULL_CHAR 1
#define MAXARGS 20
#define MATCHING_STRING 0


//I force parser to return c, because for some reason, after I call parser
//even though I would leave parser with c as -10, once I return the the 
//main after the parser call,c  would turn into 0, making my program _exit
int parser();
void execute(char*[],int);
void reroute(int);
char myBuffer[STORAGE*MAXARGS];
//char *inDirectorFile;
//char *outDirectorFile;
char *newargv[MAXARGS];
char s[STORAGE];
int c;
int i;
int j = 0;
int changeDir = 0;
int bufferCount;
int newargvCount;
int wordRank;
int saved_stdout;
int outDirectorIndex = -1;
int inDirectorIndex = -1;
int pipeIndex = -1;
int ampIndex = -1;
int parseError;
pid_t firstpid;


void myhandler(int signum){

}
/********************************************************************************************************************************/
int parser(){
	int ignoreRemainderLine;
	ignoreRemainderLine = 0;
	newargvCount = 0;
	bufferCount = 0;
	memset(myBuffer, '\0',STORAGE); 
	memset(newargv, '\0',STORAGE); 
	outDirectorIndex = -1;
	inDirectorIndex = -1;
	ampIndex = -1;	
	pipeIndex = -1;
	parseError = 0;
	//Turn c into an integer that means nothing, so I can proceed
	//into the while loop. 
	c = -100;
	while( c != NEW_LINE_SIGNAL && c != EOF_SIGNAL){
		c = getword(s);
		if( c != NEW_LINE_SIGNAL && c != EOF_SIGNAL){
			int i;
			//Check if # is the first character on the command, Check c if it came back
			//as a delimiter or a regular character|| Keep ignoring until we get out of the
			//while loop.
			if ((bufferCount == 0 && s[0] == '#' && c != 1) || ignoreRemainderLine == 1){
				ignoreRemainderLine = 1;
			}else{
				// Point newargv to where the next word starts in the myBuffer array.
				newargv[newargvCount] = &myBuffer[bufferCount];
				//If I find a delimeter than I dont want newargv to point to it!
				if (c == DELIM_SIGNAL){
					newargv[newargvCount] = NULL;
					if(strcmp(s,"<") == MATCHING_STRING){
						if(inDirectorIndex == -1)	
							inDirectorIndex = newargvCount;
						else{
							fprintf(stderr,"Ambiguous use of <\n");
							parseError = 1;
						}
					}
					if(strcmp(s,">") == MATCHING_STRING){
						if(outDirectorIndex == -1)	
							outDirectorIndex = newargvCount;
						else{
							fprintf(stderr,"Ambigious use of >\n");
						}
					}
					if(strcmp(s,"|") == MATCHING_STRING){
						pipeIndex = newargvCount;
					}
					if(strcmp(s,"&") == MATCHING_STRING){
						ampIndex = newargvCount;
						break;
					}
					if(strcmp(s,"#") == MATCHING_STRING){
						newargv[newargvCount] = &myBuffer[bufferCount];
						myBuffer[bufferCount] = s[0];
						myBuffer[bufferCount+1] = '\0';
						bufferCount++;
						bufferCount++;
					}
				}	
				//Go through each character in the token and place it consecutively in
				//myBuffer's array, incremenet the buffer so I dont fill in a spot that is
				//taken up by a previous char, even when I come back to the parse again.
				//place the Null Char as well.
				for(i=0; i<c+NULL_CHAR;i++){
					myBuffer[bufferCount] = s[i];
					bufferCount++;
					//printf("buffercount: %d\n",bufferCount);
					//printf("inserting %c\n",s[i]);
				}
				//Increment my pointer to pointers to point to the next word start in myBuffer.
				newargvCount++;
			}
		}
	}
	//This last little snippet makes sure my newargv array end with a null
	//after the command. 
	newargvCount++;
	newargv[newargvCount] = NULL;

	/**************************************************************
	int j =0;
	for(j=0;j<newargvCount;j++){
		printf("newargv[%d] is: %s\n",j,newargv[j]);
	}
	printf("newargvCount is: %d\n", newargvCount);
	****************************************************************/
	//These are the sanity checks for badly-constructed commands.
	if(newargv[pipeIndex+1] == NULL && pipeIndex != -1){
		fprintf(stderr,"|: Null Command following Pipe\n");
		parseError = 1;
	}else if(outDirectorIndex != -1 && newargv[outDirectorIndex+1] == NULL){
		fprintf(stderr,">: Redirection to Null/&/| invalid!\n");
		parseError = 1;
	}else if(inDirectorIndex != -1 && newargvCount == 2){
		fprintf(stderr,"<: Invalid Null Command!\n");
		parseError = 1;
	}
	//Even though c is global, c would change once I would leave this function.
	//So I had return c to make sure what I am leaving with here survived.
	return c;
}
/****************************************************************************/
//This function I created to handle ignoring the a director's file name when
//executing. Note in my next version this will be taken out.
void execute(char* newargvMod[],int newargvCountMod){
	int i;
	/********************************************************
	int j =0;
	for(j=0;j<newargvCountMod;j++){
		printf("newargv[%d] is: %s\n",j,newargvMod[j]);
	}
	********************************************************/
	if(s[0] == '&' && newargvMod[0] != NULL)
		printf("%s [%d]\n",newargvMod[0], getpid());
	//I actually have newargv point to the file name inside the buffer.
	//Come time to execute, I have to ignore this name and find where my 
	//command acutally is.
	for(i=0;i<newargvCountMod;i++){
		if(inDirectorIndex == -1)
			inDirectorIndex = -100;
		if(outDirectorIndex == -1)
			outDirectorIndex = -100;
		if(newargvMod[i] != NULL && newargvMod[i] != newargvMod[inDirectorIndex+1] && newargvMod[i] != newargvMod[outDirectorIndex+1]){
			// This is what was giving me such a headache with the autograder.
			// When I use execvp, anything I print in this child will NOT be
			// redirected into a file! It will however go to STDOUT. If I use
			// execv, then all of a sudden whatever this child prints, whether
			// it be STDOUT or redirected into a file, it actually does it. I
			// have spent HOURS trying to figure out why my program would print
			// to STDOOUT but not redirect into a file. It seems execvp is the 
			// culprit. I don't know how to fix this.
			CHK(execvp(newargvMod[i], newargvMod+i));
		}
	}
	//We should never reach this exit.
	_exit(10);
}
/********************************************************************************************************************************/
int main(){
	
	newargvCount = 0;
	setpgid(firstpid, 0);
	//From Carroll's Reader, "By default, your program abruptly terminates upon reciving SIGTERM
	//but the following function call CATCHES the signal and instead does something different!"
	signal(SIGTERM, myhandler);
	//printf("parent's process group id is %d\n", (int) getpgrp());
	//printf("parent's process id is %d\n", (int) getpid());
	//printf("parent's parent id is %d\n", (int) getppid());
	//printf("parent's parent group id is %d\n", (int) getpgid(getppid()));
	for(;;){
		//Issue the prompt.
		if(c != -1)
			printf("p2: ");
		// If c is 0 this means the getword.c lexical analyser encountered the EOF
		// and _exited correctly, if we have EOF we want to _exit p2.c too!
		c =parser();
		//If I have an EOF_Signal then I need to leave
		if(c == EOF_SIGNAL){
			break;
		//If my myBuffer is empty
		}else if(myBuffer[0] == '\0'){
			//do nothing.
		/*******************************************************************************************/
		//This is for the two built-in commands 	
		//I have to check if newargv is NULL because if it is than strcmp will give a seg fault because
		//in strcmp we try to dereference a null which makes my program crash.
		}else if (newargv[0] != NULL && strcmp(newargv[0],"cd") == 0){
			if(newargv[2] != NULL){
				fprintf(stderr,"cd Error: There are too many arguments\n");
			}
			else if(newargv[1] != NULL){
				chdir(newargv[1]);
			}else{
				chdir(getenv("HOME"));
			}
		}else if (newargv[0] != NULL && strcmp(newargv[0],"MV") == 0){
			if(newargv[1] == NULL){
				fprintf(stderr,"MV Error: Too few arguments\n");
			}else if(newargv[2] == NULL){
				fprintf(stderr,"MV Error: Too few arguments\n");
			}else if(newargv[3] != NULL){
				fprintf(stderr,"MV Error: Too many arguments\n");
			}else{
				int status;
				/**************************************
				char cwd[1024];
				getcwd(cwd,sizeof(cwd));
				printf("current working is: %s\n", cwd);
				/**************************************/
				//I want to check the status of the link
				//because I dont want to unlink, unless
				//I have linked because I will loose
				//both files in such a case.
				CHK(status = link(newargv[1],newargv[2]));
				//
				if (status == 0){
					CHK(unlink(newargv[1]));
				}
			}
		/*******************************************************************************************/
		}else if (parseError == 1){
			//printf("There should be a parser error!\n");
			parseError = 0;
		}else{
			fflush(stdout);
			CHK(firstpid = fork());
			//fork() returns 0 to the newly-created child process.
			if (firstpid == 0){
				//printf("child created and running...%d\n", getpid());
				//printf("child's process group id is: %d\n", (int) getpgrp());		
				//printf("child's process id is %d\n", (int) getpid());
				//My forked child plays here.
				//Look through all the token I am pointing to, if I find a redirect
				/*******************************************************************************/
				if(inDirectorIndex != -1){
					int input_fd;
					int flags;
					char *fileName = newargv[inDirectorIndex+1];
					CHK(saved_stdout = dup(1));
					flags =  O_RDONLY;
					//This open is to change the file descriptor	
					CHK(input_fd=open(fileName,flags));
					CHK(dup2(input_fd, STDIN_FILENO));
					CHK(close(input_fd));
					//If we found both < and > in the command, we want to wait to
					//redirect standard out to print fot a file.
				}
				/*******************************************************************************/
				if(outDirectorIndex != -1){
				//This section is from Carrolls reader.
					int output_fd;
					int flags;
					char *fileName = newargv[outDirectorIndex+1];
					CHK(saved_stdout = dup(1));
					//O_EXCL: Ensure that this call creates the file: if this flag 
					//is specified in conjunction with O_CREAT, and pathname already
					//exists, the open() fails with the rror EEXIST.
					//
					//Have to have O_WRONLY, I think we need to specify why we are
					//opening the file, in this case we are writing to it so I chose
					//O_WRONLY'
					flags =  O_WRONLY | O_CREAT | O_EXCL;	
					//S_IRUSR: 00400 user has read permission
					//s_IWUSR: 00200 user has write permission 
					CHK(output_fd=open(fileName,flags, S_IRUSR | S_IWUSR));
					CHK(dup2(output_fd, STDOUT_FILENO));
					CHK(close(output_fd));
					//Since we found both < and > and did not proceed to print the
					//conents of the file above because we were waiting to redirect
					//standard output, we can now commence printing since in and out
					//are directed to where we need them to go.
				}	
				/*******************************************************************************/
				//The following if checks for a pipe in the command line. If there is,
				//The child creates its own child which we redirect its standard output
				//into a pipe. When its done, the original child's input is rerouted to take
				//in information from the pipe (place by grandchild) and with that info 
				//spits its results on to the screen. Most of the code came from Carroll's
				//reader page 6, just modified for a child grandchild manner instead of two
				//children like in the reader.
				if(pipeIndex != -1){
					int filedes[2];
					int offset = 1;
					pid_t secondpid;
					CHK(pipe(filedes));
					//the left side of the pipe. This grandchild needs to finish
					//before we let its parent(the original child) continue becuase
					//it is the child that is going to print to screen. After this
					//the grandchild is no longer going to print anything
					CHK(secondpid = fork());
					if(secondpid == 0){
						//NOTHING WILL BE PRINTED TO SCREEN AFTER ABOVE DUP
						//BUT WE WILL STILL BE UP AND RUNNING
						CHK(dup2(filedes[1],STDOUT_FILENO));
						CHK(close(filedes[0]));
						CHK(close(filedes[1]));
						execute(newargv,pipeIndex);
					}else{
						//Child wait for grandchild to finish!
						wait(NULL);
						for(;;){
							int status;
							//https://stackoverflow.com/questions/5278582/checking-the-status-of-a-child-process-in-c
							//This check if ther is a child, out there, if there isn't we break from for(;;)
							pid_t result = waitpid(secondpid, &status, WNOHANG);
							if(result == 0)
								wait(NULL);
							else{
								//All children are dead.
								break;
							}
						}
					}
					CHK(dup2(filedes[0],STDIN_FILENO));
					CHK(close(filedes[0]));
					CHK(close(filedes[1]));
					//If there exists an >, then we need to redirect this processes
					//output into a file.
					execute(newargv+pipeIndex+offset, newargvCount-offset-pipeIndex );
					//I need this extra close or the child will never see EOF
					CHK(close(filedes[0]));
					CHK(close(filedes[1]));
				}//End pipe found
				else{
					int j =0;
					//printf("execute no pipe\n");
					execute(newargv,newargvCount);
				}
			//If I am the parent then I need to wait unless... [see next else]
			}else if(firstpid != 0 && s[0] == '&'){
				//printf("NOT WAITING %d\n",getpid());
			}else if(firstpid != 0){
				int status;
				wait(NULL);
				/*************************************************/
				//I might be reaping children faster than they are
				//dying. In this case, wait will continue for ANY
				//child to die to continue, IT DOESNT MATTER IF 
				//THERE ARE MORE. Wait does just that, waits for
				//any child to die to keep going.
				for(;;){
					//https://stackoverflow.com/questions/5278582/checking-the-status-of-a-child-process-in-c
					//This check if ther is a child, out there, if there isn't we break from for(;;)
					pid_t result = waitpid(firstpid, &status, WNOHANG);
					if(result == 0)
						wait(NULL);
					else{
						//All children are dead.
						break;
					}
				}
				/*************************************************/
			}
		}//End if there is an actual command.
		//printf("continuing... %d\n",getpid());
	}//End infinite Loop <--- Ha!, oxymoron
	//printf("running kill pg\n");
	killpg(getpgrp(),SIGTERM);
	printf("p2 terminated.");
	printf("\n");
	_exit(0);
}
