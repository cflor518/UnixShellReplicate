/********************************************************
 * Program 4 
 * Programmer: Cesar Flores
 * Professor: John Carroll
 * Class: CS570
 * Due Date: 4/20/2018 at 11pm.
 *
 * Program 4 : This program is an enhancement of program 2.
 * Some of the improvements made are as follows. The pipe
 * command can now handle up to ten pipes. This program
 * can also accomadte the special pipe character |& which
 * redirects not only STDOUT but STDERR as well. MV has
 * enhanced functionality, If MV is handed a path for
 * the destination, the code takes the file name of the source
 * as the name of the new destination filename. This program
 * can also hanedle things non-interactively. This is done
 * by giving p2 an argument file. p2 will commence to execute
 * the commands inside this argument file if it exists and 
 * the commands are valid. The description for program 2 is
 * below:
 * Program 2 : This program mimics a shell with very limited
 * abilities. With a lexical analyzer named getword c, the 
 * pogram is able to organize the information into it own
 * array and with the tokens it is handed by getword program.
 * This program is able to handle redirection, pipes( currently
 * only one pipe), and the ampersand which is able to throw
 * processes in the background while obtaining more commands.
 * The parser is probably the most important function in that
 * it is the orgranizer of the information which determines
 * and it greatly affects how main() runs. It handles 
 * metacharacter for special case operations.
 * ******************************************************/
#include "getword.h"
#include "p2.h"
#include "CHK.h"

//I force parser to return c, because for some reason, after I call parser
//even though I would leave parser with c as -10, once I return the the 
//main after the parser call,c  would turn into 0, making my program _exit
char myBuffer[STORAGE*MAXARGS];
//Following two variable are for my lexical analyzer, getword.c
//holds a token and word count
char *newargv[MAXARGS];
char s[STORAGE];
int c;
//Following keep count of my buffer array and my newargv 
int bufferCount;
int newargvCount;
//Sometimes I need to print back to screen in case of
//an error, I save the filedescrptor for this occasion
int saved_stdout;
//Following give me my index of where certain metacharacters
//are
int outDirectorIndex = -1;
int inDirectorIndex = -1;
int pipeIndex = -1;
int ampipeCount = -1;
int pipeCount = 0;
int pipeIndexArray[PIPE_AMT_LIMIT];
int specialPipeIndexArray[PIPE_AMT_LIMIT];
char *inDirectorFile;
char *outDirectorFile;
//Tracks if parser encountered an erro with command syntax
int parseError;
//This one is for my child
pid_t firstpid;
/*I need double the amount of spaces in my file descriptor than I have
 * * pipes because I ned space for both ends of one pipe (in and out)*/
int filedes[PIPE_AMT_LIMIT*2];
int noninteractive;
void myhandler(int signum){


}
/********************************************************************************************************************************/
int parser(){
	int ignoreRemainderLine; /*Used to keep track if I am suppose to ignore lines becaise of # char*/
	int inFileRead = 0;
	int outFileRead = 0;
	inDirectorFile = NULL;
	outDirectorFile = NULL;
	ignoreRemainderLine = 0;
	newargvCount = 0;
	bufferCount = 0;
	memset(myBuffer, '\0',STORAGE); 
	memset(newargv, '\0',STORAGE); 
	outDirectorIndex = -1;
	inDirectorIndex = -1;
	pipeIndex = -1;
	pipeCount = 0;
	parseError = 0;
	//Turn c into an integer that means nothing, so I can proceed
	//into the while loop. 
	c = -100;
	c = getword(s);
	while( c != NEW_LINE_SIGNAL && c != EOF_SIGNAL){
		int i;
		//Check if # is the first character on the command, Check c if it came back
		//as a delimiter or a regular character|| Keep ignoring until we get out of the
		//while loop.
		if ((bufferCount == 0 && s[0] == '#' && c != 1) || ignoreRemainderLine == 1){
			ignoreRemainderLine = 1;
		}else if(noninteractive == 1 && s[0] == '#' && c != 1){
			ignoreRemainderLine = 1;
		}else{
			// Point newargv to where the next word starts in the myBuffer array.
			newargv[newargvCount] = &myBuffer[bufferCount];
			// Point newargv to where the next word starts in the myBuffer array.
			if(inFileRead == 1){
				newargv[newargvCount] = NULL;
				inDirectorFile = &myBuffer[bufferCount];
				inFileRead =0;
				newargvCount--;
			}
			if(outFileRead == 1){
				newargv[newargvCount] = NULL;
				outDirectorFile = &myBuffer[bufferCount];
				outFileRead =0;
				newargvCount--;
			}
			if (c == DELIM_SIGNAL){
				newargv[newargvCount] = NULL;
				if(strcmp(s,"<") == MATCHING_STRING){
					if(inDirectorIndex == -1){
						inDirectorIndex = newargvCount;
						inFileRead = 1;
						newargvCount--;
					}
					else{
						fprintf(stderr,"Redirect Error: Ambiguous use of <\n");
						parseError = 1;
					}
				}
				if(strcmp(s,">") == MATCHING_STRING){
					if(outDirectorIndex == -1){
						outDirectorIndex = newargvCount;
						outFileRead = 1;
						newargvCount--;
					}
					else{
						fprintf(stderr,"Redirect Error: Ambigious use of >\n");
						parseError = 1;
					}
				}
				if(strcmp(s,"|") == MATCHING_STRING){
					if (pipeCount == PIPE_AMT_LIMIT){
						fprintf(stderr,"The max allowable pipes is set to: %d\n",PIPE_AMT_LIMIT);
						parseError = 1;
					}
					pipeIndex = newargvCount;
					pipeIndexArray[pipeCount] = newargvCount;
					specialPipeIndexArray[pipeCount] = REGULAR_PIPE;
					pipeCount++;
				}
				if(strcmp(s,"|&") == MATCHING_STRING){
					if (pipeCount == PIPE_AMT_LIMIT){
						fprintf(stderr,"The max allowable pipes is set to: %d\n",PIPE_AMT_LIMIT);
						parseError = 1;
					}
					pipeIndex = newargvCount;
					pipeIndexArray[pipeCount] = newargvCount;
					specialPipeIndexArray[pipeCount] = SPECIAL_PIPE;
					pipeCount++;
				}
				if(strcmp(s,"&") == MATCHING_STRING){
					newargvCount++;
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
			}
			//Increment my pointer to pointers to point to the next word start in myBuffer.
			newargvCount++;
		}
		c = getword(s);
	}//end while
	//Insanity checks for badly-constructed commands
	if(newargv[pipeIndex+1] == NULL && pipeIndex != -1){
		fprintf(stderr,"|: Null Command following Pipe\n");
		parseError = 1;
	}else if((inDirectorFile != NULL || outDirectorFile != NULL)&& newargv[0] == NULL){
		fprintf(stderr,"Redirection Error: I have a file for redirecting but I dont have any commands\n");
		parseError = 1;
	}else if(inFileRead == 1){ //Then I never applied my file to in
		fprintf(stderr,"<: Didnt recieve a file for directing!\n");
		parseError = 1;
	}
	/********************************************************************/
	//Even though c is global, c would change once I would leave this function.
	//So I had return c to make sure what I am leaving with here survived.
	return c;
}
/********************************************************************************************************************************/
int main(int argc, char *argv[]){
	noninteractive= 0;
	if(argc > 2){
		fprintf(stderr,"p2 says: I cant handle more than 1 argument when you call me.\n");
		fprintf(stderr,"Qutting to go contemplate my inadequacies.\n");
		exit(1);
	}else if(argc == 2){
		int input_fd;
		//Check if the arguments passed to p2 even exist!
		if(access (argv[1],F_OK) == -1){
			fprintf(stderr,"Non-interactive Error: The arguement you supplied to p2 does not exist: %s.\n",argv[1]);
			fprintf(stderr,"See ya later, Cap'n.\n");
			exit(1);
		}
		//If p2 was given an argument, then there is a file with commands
		//in it I need to execute, I redirect STDIN of parent from keyboard
		//to file.
		CHK(input_fd=open(argv[1],O_RDONLY, S_IRUSR|S_IWUSR));
		dup2(input_fd,STDIN_FILENO);
		close(input_fd);
		noninteractive = 1;
	}
	newargvCount = 0;
	setpgid(firstpid, 0);
	//From Carroll's Reader, "By default, your program abruptly terminates upon reciving SIGTERM
	//but the following function call CATCHES the signal and instead does something different!
	signal(SIGTERM, myhandler);
	for(;;){
		if(noninteractive == 0){
			printf("p2: ");
		}
		// If c is 0 this means the getword.c lexical analyser encountered the EOF
		// and _exited correctly, if we have EOF we want to _exit p2.c too!
		c =parser();
		//To exit my program when I see EOF Signal
		if(c == EOF_SIGNAL){
			break;
		//If my myBuffer is empty
		}else if(myBuffer[0] == '\0'){
			//do nothing.
		/***************************************************************************
		//The are the built in command MV and cd are handled by the parent. Inside
		//we do insanity checks.
		//
		//I have to check if newargv is NULL because if it is than strcmp will give a seg fault because
		//in strcmp we try to dereference a null which makes my program crash.
		***************************************************************************/
		}else if (newargv[0] != NULL && strcmp(newargv[0],"cd") == 0){
			if(newargv[2] != NULL){
				fprintf(stderr,"cd Error: There are too many arguments\n");
			}
			else if(newargv[1] != NULL){
				chdir(newargv[1]);
			}else{
				chdir(getenv("HOME"));
			}
		/***************************************************************************/
		}else if (newargv[0] != NULL && strcmp(newargv[0],"MV") == 0){
			DIR* dir;
			DIR* dir1;
			char source[50];
			char destination[50];
			int status;
			int flagToHonor;
			int sourceIndex;
			int destinationIndex;
			int argument;
			int errorDuringMVSetUp_NoDirectoryDest;
			int errorDuringMVSetUp_TooManyArgs;
			int errorDuringMVSetUp_TooFewArgs;
			char destinationPath[50];
			char sourceFileName[50];
			char * destinationSlashIndex;
			char * sourceSlashIndex;
			sourceIndex = 0;
			destinationIndex = 0;
			argument = 1;
			errorDuringMVSetUp_NoDirectoryDest = 0;
			errorDuringMVSetUp_TooManyArgs = 0;
			errorDuringMVSetUp_TooFewArgs = 0;
			/**************************************************************************************/
			//FOR FLAGS
			// The move commands might have a bunch of flags behind them, the following finds
			// where the actual source and destination names are.
			do{
				if(newargv[argument][0] != '-' && sourceIndex != HAVENT_FOUND_IT && destinationIndex != HAVENT_FOUND_IT){
					errorDuringMVSetUp_TooManyArgs = 1;
				}
				if(newargv[argument][0] != '-' && destinationIndex == HAVENT_FOUND_IT && sourceIndex != HAVENT_FOUND_IT){
					strcpy(destination,newargv[argument]);
					destinationIndex = argument;
				}
				if(newargv[argument][0] != '-' && sourceIndex == HAVENT_FOUND_IT){
					strcpy(source,newargv[argument]);
					sourceIndex = argument;
				}
				argument++;
			}while(newargv[argument] != NULL);
			if(sourceIndex == HAVENT_FOUND_IT || destinationIndex == HAVENT_FOUND_IT){
				errorDuringMVSetUp_TooFewArgs = 1;
				sourceIndex = 1;
				destinationIndex = 1;
			}
			/**************************************************************************************/
			//FOR FLAGS
			// There might mutiple flags behind the source or the destination names. The following
			// code picks which flag to honor and execute.
			if(newargv[sourceIndex-1][0] == '-' && newargv[sourceIndex-1][0] != '-'){
				flagToHonor = sourceIndex-1;
			}else if(newargv[sourceIndex-1][0] == '-' && newargv[sourceIndex-1][0] == '-'){
				flagToHonor = destinationIndex-1;
			}else{
				flagToHonor = destinationIndex-1;
			}
			/*************************************************************************************/
			//Figure out what the actual filename is of the source in case its a directory plus a filename
			//and not just a file name. 
			sourceSlashIndex = strrchr(newargv[sourceIndex],'/');
			if(sourceSlashIndex != NULL){
				int sourceLastSlashIndex = sourceSlashIndex - newargv[sourceIndex];
				int n;
				int k;
				n=sourceLastSlashIndex+1;
				k=0;
				while(newargv[sourceIndex][n] != '\0'){
					sourceFileName[k] = newargv[sourceIndex][n];
					n++;
					k++;
				}
				//Make sure the appended to destination ends in a null character
				sourceFileName[k] = '\0';
			}else{
				strcpy(sourceFileName,newargv[sourceIndex]);
			}
			/*************************************************************************************/
			// For the destination, If it a path attached with a filename, I need to remvoe the 
			// filename and see if it is a valid path.
			destinationSlashIndex = strrchr(newargv[destinationIndex],'/');
			if(destinationSlashIndex != NULL){
				int destinationLastSlashIndex = destinationSlashIndex - newargv[destinationIndex];
				int m;
				for(m=0;m<destinationLastSlashIndex;m++){
					destinationPath[m] = newargv[destinationIndex][m];
				}
				//Make sure the appended to destination ends in a null character
				destinationPath[m] = '\0';
			}else{
				strcpy(destinationPath,newargv[destinationIndex]);
			}
			/*************************************************************************************/
			//Append the source's file name to the destination if the given destination is 
			//a directory. Got the following code to check if directory from:
			//https://stackoverflow.com/questions/12510874/how-can-i-check-if-a-directory-exists
			//Also, found out that ENOENT means Error,NO ENTity.
			dir = opendir(newargv[destinationIndex]);
			if (dir){
			//The directory exists, so append source name to the end of destination path.
				strcat(destination,"/");	
				strcat(destination,sourceFileName);	
				closedir(dir);
			}
			else if (ENOENT == errno){
				//It is not a valid path, but it might be a valid path with a 
				//non existent file name attached, check for this.	
				dir1 = opendir(destinationPath);
				if (dir1){
					//Detaching the file name from the destination
					//proves that it is a valid path.
				}
				else if (ENOENT == errno){
				//The original destination is not a path, and it 
				//is not a valid path with non existent file attached.
					if(errorDuringMVSetUp_TooManyArgs == 0 && errorDuringMVSetUp_TooFewArgs == 0){
						errorDuringMVSetUp_NoDirectoryDest = 1;
					}
				}
			}
			/*************************************************************************************/
			// Sanity Checks for Errors.
			if (errorDuringMVSetUp_NoDirectoryDest == 1){
				fprintf(stderr,"MV Error: Destination %s is not a valid directory\n",newargv[destinationIndex]);
			}else if (errorDuringMVSetUp_TooManyArgs == 1){
				fprintf(stderr,"MV Error: Found too MANY arguments!\n");
			}else if (errorDuringMVSetUp_TooFewArgs == 1){
				fprintf(stderr,"MV Error: Found too FEW arguments!\n");
			}else if(newargv[1] == NULL){
				fprintf(stderr,"MV Error: Too few arguments\n");
			}else if(newargv[2] == NULL){
				fprintf(stderr,"MV Error: Too few arguments\n");
			}else if(access (newargv[sourceIndex],F_OK) == -1){
			//File does not exists
				fprintf(stderr,"MV Error: You some sort of psycho? Source file %s does not exists!\n",newargv[sourceIndex]);
			/**********************************************************************/	
			//Check for the -n flag, if it is there dont clobber if it exist
			}else if(strcmp(newargv[flagToHonor],"-n") == 0){	 	
				//Check if file exists so I dont clobber it 
				if(access (destination,F_OK) != -1){
					//file exists!
					fprintf(stderr,"MV Error: You told me not to clobber: %s\n",newargv[destinationIndex]);
				}
			/**********************************************************************/	
			//Check for the -f flag, dont say youre overwriting just overwrite 
			}else if(strcmp(newargv[flagToHonor],"-f") == 0){	 	
				//printf("You told me to clobber %s so Im clobbering\n",newargv[destinationIndex]);
				if(access (destination,F_OK) != -1){
					CHK(unlink(destination));
				}
				CHK(status = link(newargv[sourceIndex],destination));
				if (status == NO_PROBLEM){
					CHK(unlink(newargv[sourceIndex]));
				}else{
					fprintf(stderr,"MV Error: Something when wrong with the linking to: %s\n",newargv[destinationIndex]);
				}
			/**********************************************************************/	
			}else{
				//I want to see if link was successfil
				//because it wasn't I should not delete
				//what I tried to just link. This would
				//delete the file instead of moving it.
				CHK(status = link(source,destination));
				if (status == 0){
					CHK(unlink(source));
				}
			}
		/***************************************************************************/
		//If i have an error parsing the command given to me then I dont need to
		//execute anything and need to go back and ask for a reasonable command.
		}else if (parseError == 1){
			parseError = 0;
		}else{
			int childP;
			fflush(stdout);
			CHK(firstpid = fork());
			//Need parent to know what his child's pid is.
			childP = firstpid;	
			//fork() returns 0 to the newly-created child process.
			if (firstpid == 0){
				//My forked child plays here.
				/*******************************************************************************/
				//Redirect in if you found such a request in the command
				if(inDirectorIndex != NOT_FOUND){
					int input_fd;
					int flags;
					saved_stdout = dup(1);
					flags =  O_RDONLY;
					//This open is to change the file descriptor	
					CHK(input_fd=open(inDirectorFile,flags));
					CHK(dup2(input_fd, STDIN_FILENO));
					CHK(close(input_fd));
				/*******************************************************************************/
				//If you didnt find anything to redirect IN from then I want to take from
				//Null, so that you dont compete for keystrokes with anyone else.
				}else{
					int null_fd;
					int flags;
					flags =  O_RDONLY;
					//This open is to change the file descriptor	
					CHK(null_fd=open("/dev/null",flags));
					CHK(dup2(null_fd, STDIN_FILENO));
					CHK(close(null_fd));
				}
				/*******************************************************************************/
				if(outDirectorIndex != NOT_FOUND){
				//This section is from Carrolls reader.
					int output_fd;
					int flags;
					saved_stdout = dup(1);
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
					CHK(output_fd=open(outDirectorFile,flags, S_IRUSR | S_IWUSR));
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
				if(pipeIndex != NOT_FOUND){
					int timesExecuted =1;
                                        CHK(pipe(filedes));
                                        forkPlummerChildren(timesExecuted);
				}//End pipe found
				else{	
					CHK(execvp(newargv[0],newargv));
				}
			//If I am the parent then I need to wait unless... [see next else]
			}else if(firstpid != 0 && s[0] == '&'){
				//Need to print the background process of the children.
				if(s[0] == '&' && newargv[0] != NULL)
					printf("%s [%d]\n",newargv[0], childP);
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
	}//End infinite Loop <--- Ha!, oxymoron
	killpg(getpgrp(),SIGTERM);
	printf("p2 terminated.\n");
	exit(0);
}
void forkPlummerChildren(int execCount){
	/*The following two variables are to assign the appropriate pipe ends to
	the child that is running this code. Which grandchild we are dealing with
	is deducted by the number of times this recursive function has run. The 
	leftmost child (the greatest grandchild) will get the pipe ends furthest
	down the filedescriptorarray named filedes*/ 
        int pipeOut = (execCount*PIPE_ENDS)-PIPE_OUT;
        int pipeIn = (execCount*PIPE_ENDS)-PIPE_IN;
        pid_t pid;
        if((pid = fork()) == 0){
		//We will most likely be forking another child, in this case we create a pipe
		//for the upcoming potential forked child. 
                CHK(pipe(filedes+(execCount*PIPE_ENDS)));
                //If how many times I run is not equal to the amount of pipes I have in the command
                if(execCount != pipeCount){
                        //recursively call yourself so you can go fork another child to try to pipe another command(if necessary).
                        forkPlummerChildren(execCount+1);
                }else{
		//If you enter here than you are the greatest granchild and
		//need to execute the leftmost commmand.
                        CHK(dup2(filedes[pipeOut],STDOUT_FILENO));
			//I have a special array called specialPipeIndexArray that keeps
			//tabs on where special pipes (|&) are located inside my 
			//arrray that keeps tab on where pipes are located in the 
			//newargv array.
			if(specialPipeIndexArray[0] == SPECIAL_PIPE){
				CHK(dup2(filedes[pipeOut],STDERR_FILENO));
			}
                        CHK(close(filedes[pipeOut]));
                        CHK(close(filedes[pipeIn]));
                        execvp(newargv[0], newargv);
                }
        }
	//Parent of above's child plays here.
        CHK(dup2(filedes[pipeIn],STDIN_FILENO));
        CHK(close(filedes[pipeIn]));
        CHK(close(filedes[pipeOut]));


        /* pipeIndexArray[integer]+OFFSET: is an array to hold the indexed location of the pipe symbol
        * inside the my newargv array, the OFFSET is to go one index up to give newargv the location of
        * the command after the pipe.
        *
        *  [abs(execCount-pipeCount)]+OFFSET]: Depending on which kid I am I need to find the index that belongs to me in the 
        * pipeArrayIndex to run the right command. For example, if I have 4 pipes, the second greatest grandchild needs to	
        * obtain the number at index 0 in the pipeArray index this will give the right index for use in the newargv array,
        * the third greatest grandhcild needs to obtain the number at index 1.
        * The absolute value of: the number of times the function has run minus the pipeCount is giving the right indicies. How I
        * figured that out is a mystery (pen and paper) that took me quite sometime.*/
        if(execCount != 1){
	//If you enter here, then your some middle child.	
		//Here I begin to redirect the output of the next pipe that is to be dealt with.
                CHK(dup2(filedes[pipeOut-NEXT_PIPE],STDOUT_FILENO));
		/**************************************************/
		//Checks to see if the next pipe is going to need to redirect also the STDERR 
		if(specialPipeIndexArray[abs(execCount-pipeCount)]== SPECIAL_PIPE){
			CHK(dup2(filedes[pipeOut-NEXT_PIPE],STDERR_FILENO));
		}
		/**************************************************/
                CHK(close(filedes[pipeOut-NEXT_PIPE]));
                CHK(close(filedes[pipeIn-NEXT_PIPE]));
                execvp(newargv[pipeIndexArray[abs(execCount-pipeCount)]+OFFSET],  newargv+pipeIndexArray[abs(execCount-pipeCount)]+OFFSET);
        }else{
	//This is the original child to run, the top ancestor, needs to run the rightmost command.  
                execvp(newargv[pipeIndexArray[pipeCount-OFFSET]+OFFSET],  newargv+pipeIndexArray[pipeCount-OFFSET]+OFFSET);
        }
        //I need this extra close or the child will never see EOF
        CHK(close(filedes[pipeIn]));
        CHK(close(filedes[pipeOut]));
}
