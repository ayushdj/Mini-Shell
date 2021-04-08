// Modify this file for your assignment

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "guessingGame.h"

// defining global constants
#define MAX_BUFFER_SIZE 80

void sigint_handler(int sig) {
	write(1," mini-shell terminated\n",35); 
	exit(0);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// Helper Functions:
// 1. changeDirectory - method for changing directory

// This function takes in a line from the 
// fgets method and returns an edited line.
// It also does the majority of the CD work.
void changeDirectory(char* line) {

	// getting rid of the new line character
	char* fullPath = strtok(line, "\n");

	// if the user only enters "cd " or "cd" 
	if(strcmp(fullPath, "cd") == 0 || strcmp(fullPath, "cd ") == 0) { 
		printf("Path Not Specified\n");
		return;
	}

	// extracting out the path
	else {
		fullPath += 3;
		int changed = chdir(fullPath);
		if(changed == -1) {
			printf("Invalid Directory. Please Re-Enter\n");
			return;
		}
		else {
			return;
		}
	}		
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

int main(){
	
	// creating an array of built in commands
	char* builtInCommands[4] = {"cd\n", "help\n", "exit\n", "guessingGame\n"};

	char line[MAX_BUFFER_SIZE];

	while(1) {
		signal(SIGINT, sigint_handler);

		printf("mini-shell> ");
		fgets(line, MAX_BUFFER_SIZE, stdin);

		// changing to the previous directory
		if (strncmp(line, "cd", 2) == 0) {
			// getting rid of the new line character at the end
			changeDirectory(line);
		} 
		
		else if (strcmp(line, builtInCommands[1]) == 0) {
			int i;
			printf("These are the built-in commands:\n");
			for(i = 0; i < 4; i++) {
				printf("--->\t%s", builtInCommands[i]);
			}
		}
		else if (strcmp(line, builtInCommands[2]) == 0) {
			exit(1);
		}

		else if(strcmp(line, "clear\n") == 0) {
			system("clear");
		}
		else if(strcmp(line, builtInCommands[3]) == 0) {
			playGame();
			while ((getchar()) != '\n');
		}
			
		else {	
			// ---------------------------------------------- //
			// Text Pre-Processing to see if they contain a pipe
			// command or not
			char* pipeYesOrNo = strchr(line, '|');
			char* myargv[25];
			
			// Initializing the arrays needed for a pipe command
			char* myargv_PIPE[25];
			char* pipeLEFT[25];
			char* pipeRIGHT[25];

			// If they just put in a regular command without a pipe
			if (pipeYesOrNo == NULL) {
				char* actualCommand = strtok(line, " \t\n");
				int i = 0;
				while(actualCommand != NULL) {
					myargv[i] = actualCommand;
					actualCommand = strtok(NULL, " \t\n");
					i++;
				}
				myargv[i] = NULL;	
			}

			// if the input does contain a pipe command
			if (pipeYesOrNo != NULL) {
				char* actualCommand = strtok(line, " \t\n");
				int j = 0;
				while(actualCommand != NULL) {
					myargv_PIPE[j] = actualCommand;
					actualCommand = strtok(NULL, " \t\n");
					j++;	
				}

				myargv_PIPE[j] = NULL;

				// creting the left side array of the pipe
				int k = 0;
				while(myargv_PIPE[k] != NULL) {
					pipeLEFT[k] = myargv_PIPE[k];
					if (strcmp(myargv_PIPE[k], "|") == 0) {
						break;
					}
					k++;			
				}
				pipeLEFT[k] = NULL;

				// creating the right side array of the pipr
				int l = 0;
				while(myargv_PIPE[l] != NULL) {
					if(strcmp(myargv_PIPE[l], "|") == 0) {
						pipeRIGHT[l - k] = myargv_PIPE[l+1];
					}
					l++;
				}
				pipeRIGHT[l - (k+1)] = NULL;					
			}
			
			// If they don't have the pipe in the command,
			// then do this:	
			if (pipeYesOrNo == NULL) {
				// Basic Forking Stuff
				pid_t childProcessID;
				int child_status;
				childProcessID = fork();	

				// Checking if the fork failed. If it does, 
				// we don't proceed any further
				if(childProcessID == -1) {
					printf("fork failed for some reason!");
					exit(EXIT_FAILURE);
				}
				
				// If everything works
				if (childProcessID == 0) {
					execvp(myargv[0], myargv);
	
					// If it doesn't execute, then they didn't specify a
					// correct path
					if(execvp(myargv[0], myargv) == -1) {
						printf("Command not found--Did you mean something else?\n");
						exit(0);
					}

					exit(0);			
				}
					
				else {
					wait(&childProcessID);
				}
			}

			 //piping our program	
			else if (pipeYesOrNo != NULL) {	

				// creating file descriptor array
				int fd[2];	
				pipe(fd);
				pid_t newChildPID = fork();
				int child_status_;
				pid_t secondPID;				
				secondPID = fork();
				if (secondPID == -1) {
					printf("second fork failed for some reason.\n");
					exit(EXIT_FAILURE);
				}
				
								
				// making sure that if the newChildPID
				// is negative, it is handled properly 
				if(newChildPID == -1) {
					printf("fork failed for some reason!");
					exit(EXIT_FAILURE);
				}

				if(newChildPID == 0) {
					close(STDOUT_FILENO);
					dup2(fd[1], STDOUT_FILENO);
					close(fd[1]);
       		 			close(fd[0]);
					execvp(pipeLEFT[0], pipeLEFT);
					printf("Command not found--Did you mean something else?\n");
					exit(0);
				}

				else if (secondPID == 0) {
					close(STDIN_FILENO);
					dup2(fd[0], STDIN_FILENO); 
					close(fd[1]);
					close(fd[0]);
					execvp(pipeRIGHT[0], pipeRIGHT);
					printf("Command not found--Did you mean something else?\n");
					exit(0);
				}

				// parent process
				else {
					close(fd[1]);
					close(fd[0]);
					//waitpid(newChildPID,&child_status_, child_status_);
					//waitpid(secondPID, &child_status_, child_status_);
					wait(&newChildPID);
					wait(&secondPID);
				}	
			
			}	
		}

	}
	return 0;
}
