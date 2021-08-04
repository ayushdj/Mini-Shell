// Add your program here!
// Ayush Dhananjai
// CS 5008/5009
// Homework Assignment #1

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int playGuessingGame() {

        // Initializing a guess counter
        // for the number of guesses in
        // each round
        int guessCounter = 0;

        // Beautifying
        printf("\n==================== \n");
        printf("CPU Says: Pick a number  1-10 \n");
        printf("==================== \n");

        // Initializing a random integer
        srand(time(NULL));
        int randomNumber = (rand() % (10 - 1 + 1)) + 1;
	
        while(1){
                // Asking the user to
                // enter their guess
                int userInput;
                printf("Make a guess: ");
                scanf("%d", &userInput);

                // Doing conditional checks to see if the
                // user has guessed the number correctly

                // Condition for if the guess is greater
                // than the random number generated                        
                if(userInput > randomNumber) {
                        printf("Too high! Try again! \n");
                        guessCounter++;
                }

                // Condition for if the guess
                // is less than the random number
                else if (userInput < randomNumber) {
                        printf("Too low! Try again! \n");
                        guessCounter++;
                }

                // Condition for if guess equals the
                // random number generated
                else {
                        printf("You got it! Nicely done! \n");
                        guessCounter++;

                        // Adding the number of guesses taken 
                        // in the current game to an array 
                        break;
                }
        }
        
        // Return the guess counter;
        return guessCounter;
}


void playGame() {

	// Initializing an empty constant
        // and an empty array 
        int ARRAY_SIZE = 3;
        int guessCountsTracker[ARRAY_SIZE];

        // Appending the number of guesses 
        // to an array to be temporarily stored
        for(int i = 0; i < ARRAY_SIZE; i++) {
                guessCountsTracker[i] = playGuessingGame();
        }

        printf("\n================================================= \n");
        printf("| Here are the results of your guessing abilities |");
        printf("\n================================================= \n");

        
        // Printing the number of guesses the user
        // took in each attempt of the game.
        for(int i = 0; i < ARRAY_SIZE; i++) {
                printf("Game %d took you %d guesses \n", i, guessCountsTracker[i]);
        }
	return;
}
