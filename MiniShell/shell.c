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

	// Creating a char array to set the max size of a command
	char line[MAX_BUFFER_SIZE];

	// enter an infinite while loop unless someone enters "exit"
	while(1) {
		// setting up a signal handler
		signal(SIGINT, sigint_handler);

		// printing the prompt and reading a line from the user
		printf("mini-shell> ");
		fgets(line, MAX_BUFFER_SIZE, stdin);

		// if the user wants the change directory
		if (strncmp(line, "cd", 2) == 0) {
			// getting rid of the new line character at the end
			changeDirectory(line);
		} 
		// if the user enters "help"
		else if (strcmp(line, builtInCommands[1]) == 0) {
			int i;
			printf("These are the built-in commands:\n");
			for(i = 0; i < 4; i++) {
				printf("--->\t%s", builtInCommands[i]);
			}
		}
		// if the user enters "exit" and wants to exit out of the minishell
		else if (strcmp(line, builtInCommands[2]) == 0) {
			exit(1);
		}
		// if the user enters clear to clear the screen
		else if(strcmp(line, "clear\n") == 0) {
			system("clear");
		}

		// if the user wants to play the built-in game
		else if(strcmp(line, builtInCommands[3]) == 0) {
			playGame();
			while ((getchar()) != '\n');
		}
		
		// if the user enters a normal command like ls or pwd or even a pipe command	
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

			// if the input does contain a pipe command, then we need to initialize the
			// arrays needed to execute a pile command.
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
			// then execute normally:	
			if (pipeYesOrNo == NULL) {
				// create a child processID
				pid_t childProcessID;
				int child_status;
				// fork to create a child process
				childProcessID = fork();	

				// Checking if the fork failed. If it does, 
				// we don't proceed any further
				if(childProcessID == -1) {
					printf("fork failed for some reason!");
					exit(EXIT_FAILURE);
				}
				
				// If we successfully created the child process and we are
				// currently in the child process
				if (childProcessID == 0) {
					// execute the child process
					execvp(myargv[0], myargv);
	
					// If it doesn't execute, then it means that the command most likely doesn't
					// exist 
					if(execvp(myargv[0], myargv) == -1) {
						printf("Command not found--Did you mean something else?\n");
						exit(0);
					}

					exit(0);			
				}
				// If we are in the parent process, then we wait on the child process
				// to finish and then we continue w/ the parent process.	
				else {
				
					wait(&childProcessID);
				}
			}

			// if we entered a pipe command, then we enter here.	
			else if (pipeYesOrNo != NULL) {	

				// creating file descriptor array
				int fd[2];	
				// we pipe on the file descriptor array
				pipe(fd);

				// forking to create a child process for the commands on the left side of 
				// the pipe command
				pid_t newChildPID = fork();
				int child_status_;
				
				pid_t secondPID;
				// forking to create a child process for the commands on the RHS of the pipe command				
				secondPID = fork();
				// basic check to see if we were actually able to fork
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
				// if we're in the child process for the LHS of the pipe command, we
				// we close the STDOUT_FILENO, duplicates the file descriptor at fd[1] to STDOUT_FILENO.
				// Essentially telling the STDOUT_FILENO to catch the output of the command on the left 
				// and not display it to the screen.
				if(newChildPID == 0) {
					close(STDOUT_FILENO);
					dup2(fd[1], STDOUT_FILENO);
					// closing the file descriptors
					close(fd[1]);
       		 			close(fd[0]);
					// executing the command on the LHS of the pipe command
					execvp(pipeLEFT[0], pipeLEFT);
					// if the command didn't execute, then the command doesn't exist so we 
					// let the user know and then exit from the child process.
					printf("Command not found--Did you mean something else?\n");
					exit(0);
				}
				// If we're in the child process for the RHS of the pipe command, we do the same
				// as we did for the child process for the LHS. we redirect the STDIN_FILENO to look
				// for input from the file descriptor array, instead of the user
				else if (secondPID == 0) {
					close(STDIN_FILENO);
					dup2(fd[0], STDIN_FILENO);
					// closing the file descriptors
					close(fd[1]);
					close(fd[0]);
					// executing the RHS of the pipe command
					execvp(pipeRIGHT[0], pipeRIGHT);
					// if the command didn't execute, then the command doesn't exist so we let 
					// the user know and then exit from this child process
					printf("Command not found--Did you mean something else?\n");
					exit(0);
				}

				// parent process. Closing the file descriptor array and then waiting on the 
				// two child processes
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
