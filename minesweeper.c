# include <stdio.h>
#include <time.h>

typedef struct {
    int puzzle[9][9]; //the current status of the puzzle
    int solution[9][9]; //the compleate solution of the game
    int initial[9][9]; //the initial state of the puzzle, to track the fixed cells
    time_t start_time; //tracks when the game starts
} SudokuGame;

// hej får du upp denna 

