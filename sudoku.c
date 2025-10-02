#include "sudoku.h"
#include <stdio.h> // For testing, remove when using VGA

#ifndef SUDOKU_SIZE
#define SUDOKU_SIZE 9
#endif

void sudoku_play(void) {
    int grid[SUDOKU_SIZE][SUDOKU_SIZE] = {0}; // Initialize a 9x9 grid with zeros

    // Just print the empty grid for now (replace with VGA later)
    for(int i = 0; i < SUDOKU_SIZE; ++i) {
        for(int j = 0; j < SUDOKU_SIZE; ++j) {
            printf("%d ", grid[i][j]);
        }
        printf("\n");
    }
}