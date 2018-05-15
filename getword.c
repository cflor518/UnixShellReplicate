 /********************************************************
 * getword.c - This program is a lexical analyzer. This program
 * gets repeatedly called by a driver program p1.c. The driver
 * program keeps calling until getword.c return a certain value
 * signifying it has read a EOF signal. This program pays attention
 * to certain symbols and symbol combinations to notify the driver
 * we have encountered a special lexeme. Return the size of the word
 * (unless a special word is encountered) * and overwrites a string
 * contained in p1.c. So in a sense it returns two things.
 *
 * Program 1
 * Programmer: Cesar Flores
 * Professor: John Carroll
 * Class: CS570
 * Date Started: 1/26/18
 * Last Updated: 2/9/2018
 * Due Date: 2/9/2018 at 11pm.
 * ******************************************************/
#include "getword.h"
#include <string.h>

int getword(char *word){
	int iochar;
	int count = 0;
	int skipMetaChar;
	char buffer[STORAGE];
	memset(word, '\0',STORAGE); //Clears the mem location of word.
	while((iochar = getchar()) != EOF){ //This little snippet I borrow from CbyDiscovery
		buffer[count] = iochar;
		skipMetaChar = 0;
		count++;
		if(count > STORAGE-2){
			strncpy(word,buffer,(size_t)(count));
			return count;
		}
		/*****************************************************************************************/
		if(iochar == ' '){
			//If you find a space in the stream and it isnt the first thing
			//in the buffer then it is a delimter. Return whatever you have
			//in the buffer my word.
			if(count != 1){
				strncpy(word,buffer,(size_t)(count-1));
				return count-1;
			}else{
				//Need to ignore leading spaces, so I sequentially
				//sift through the spaces and reassign once I arrive
				//at a non-space.
				while(iochar == ' '){
					iochar = getchar();
				}
				buffer[count-1] = iochar;
			}
		}
		/*****************************************************************************************/
		//If the newline is the first thing in this buffer return -10
		if(iochar == '\n'){
			if(count == 1)
				return -10;
			// Since we have arrived at our newline character and I have "eaten" it, I
			// need to put it back so I can run getword() on the newline char that 
			// made me enter this block.
			ungetc('\n',stdin);
			strncpy(word,buffer,(size_t)(count-1));
			return count -1;
		}
		/*****************************************************************************************/
		//If I find a backslash, check for the next character in the input and check for a
		//metacharacter, if one is found, ignore the backslash and place the metacharacter inside
		//the buffer and continue with the word as if the metacharacter were not a delimter.
		if(iochar == '\\'){
			int iocharNext = getchar();
			if(iocharNext == '\\' || iocharNext == '<' || iocharNext == '>' ||iocharNext == '|'|| iocharNext == '&' || iocharNext == '#'||iocharNext == ' '){	
				buffer[count-1] = iocharNext;
				iochar = iocharNext;
				skipMetaChar = 1;
			//If the next character after the backslash is a newline,
			//than put the newline back on the stream and dont count
			//the backslash in the buffer
			}else if(iocharNext == '\n'){
				ungetc(iocharNext,stdin);
				count--;
			//Same as preceeding comment only replace newline with EOF.
			}else if(iocharNext == EOF){
				ungetc(iocharNext,stdin);
				count--;
			//If you dont find a metacharacter or a newline then just 
			//place whatever you found after the backslahs and throw 
			//it in the buffer
			}else{
				buffer[count-1] = iocharNext;
			}
		}
		/*****************************************************************************************/
		//This section of code check for the metacharacter combination |&
		if(iochar == '|'){
			if(count == 1){
				int iocharNext = getchar();
				//If there is no backlash I then check if the following char after
				//the pipe is an ampersand.
				if(iocharNext == '&'){
					word[0] = '|';
					word[1] = '&';
					return -2;
				}else{
				//If the next character after the pipe is not ampersand
				//put whatever I found back into the stream what was
				//after the pipe 
					ungetc(iocharNext,stdin);
				}
			}
		}
		/*****************************************************************************************/
		//Try for any of the metacharacters, If I am told that metacharacter for this run isnt a 
		//delimter than I dont not enter here.
		if((iochar == '<' || iochar == '>' ||iochar == '|'|| iochar == '&' || iochar == '#') && skipMetaChar != 1){	
			//If this metacharacter is the first in the word buffer than it itself is a word.
			if(count == 1){
				strncpy(word,buffer,(size_t)(count));
				return -1;	
			}
			//If not print what you have in the buffer so far and and unget yourself so
			//you can be your own word in the next run of getword().Please note that the
			//# symbol is a special, special case; extra special, if you will.
			if(count > 1 && iochar != '#'){
				strncpy(word,buffer,(size_t)(count-1));
				ungetc(iochar,stdin);
				return count-1;
			}
		}
}
	// When there is no newline at the end I need a way to
	// print what is already in my buffer when I encounter
	// the EOF signal. This if statement does this.
	if(iochar == EOF && count > 1){
		strncpy(word,buffer,(size_t)(count));
		//Once I print the word that its in the buffer,
		//I need to put my EOF signal back into the input.
		ungetc(EOF,stdin); 
		return count;
	}
	return 0;
}
