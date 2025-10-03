#include "sudoku.h"
#include "sudoku_puzzles.h" // Include predefined puzzles, remove when using VGA
#include <stdio.h> // For testing, remove when using VGA
#include <stdlib.h> // For rand()
#include <time.h> // For time tracking
#include <string.h> // For memset (used to set a block of memory to a specific value, typically zero)


// Function to initialize the Sudoku game
void sudoku_init(SudokuGame *game, SudokuDifficulty difficulty) {
    memset(game, 0, sizeof(SudokuGame)); // Clear the game structure, set all values to 0
    game->difficulty = difficulty; // Set the difficulty level
    game->state = GAME_RUNNING; // Set initial game state to running
    game->start_time = time(NULL); // Record the start time
    game->elapsed_time = 0.0;   // Initialize elapsed time to 0, will be updated later in the game loop 
    game->selected_row = 0;     // Start with the first cell selected, top-left corner
    game->selected_col = 0;

// Select a random puzzle based on difficulty
    int num_puzzles;
    if (difficulty == EASY) {
        num_puzzles = NUM_EASY;
    } else if (difficulty == MEDIUM) {
        num_puzzles = NUM_MEDIUM;
    } else {
        num_puzzles = NUM_HARD;
    }
    int puzzle_index = rand() % num_puzzles;

// Pointer to the selected puzzle
    const int (*selected_puzzle)[9][9]; 
    if (difficulty == EASY) {  
        selected_puzzle = &easy_puzzles[puzzle_index];
    } else if (difficulty == MEDIUM) {
        selected_puzzle = &medium_puzzles[puzzle_index];
    } else {
        selected_puzzle = &hard_puzzles[puzzle_index];
    }

// Initialize the game grid with the selected puzzle
    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            int value = (*selected_puzzle)[r][c];
            game->grid.cells[r][c].value = value;
            if (value != 0) {
                game->grid.cells[r][c].fixed = 1; // Mark as fixed if it's part of the initial puzzle
            } else {
                game->grid.cells[r][c].fixed = 0; // Not fixed, can be changed by the player
            }
        }
    }
}   

