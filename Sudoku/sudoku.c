#include "sudoku.h" 
#include "sudoku_puzzles.h" // Include predefined puzzles, remove when using VGA
#include <stdio.h> // For testing, remove when using VGA
#include <stdlib.h> // For rand()
#include <time.h> // For time tracking
#include <string.h> // For memset (used to set a block of memory to a specific value, typically zero)

// shuffle function to randomize puzzle selection
void swap_rows(int grid[9][9], int row1, int row2) {  
    for (int col = 0; col < 9; col++) {
        int temp = grid[row1][col];
        grid[row1][col] = grid[row2][col];
        grid[row2][col] = temp; 
    }
}

// Function to swap columns
void swap_cols(int grid[9][9], int col1, int col2) { 
    for (int row = 0; row < 9; row++) {
        int temp = grid[row][col1];
        grid[row][col1] = grid[row][col2];
        grid[row][col2] = temp;
    }
}

// Function to shuffle rows within each band (set of 3 rows)
void shuffle_rows(int grid[9][9]) { 
    for (int band = 0; band < 3; band++) {
        int base = band * 3;
        for (int i = 0; i < 3; i++) {
            int j = rand() % 3;
            swap_rows(grid, base + i, base + j);
        }
    }
}

// Function to shuffle columns within each stack (set of 3 columns)
void shuffle_cols(int grid[9][9]) { 
    for (int stack = 0; stack < 3; stack++) {
        int base = stack * 3;
        for (int i = 0; i < 3; i++) {
            int j = rand() % 3;
            swap_cols(grid, base + i, base + j);
        }
    }
}

// Function to permute numbers in the grid, maps 1-9 to a random permutation of 1-9
void permute_numbers(int grid[9][9]) {
    int map[10] = {0};
    for (int i = 1; i <= 9; i++) map[i] = i;
    for (int i = 1; i <= 9; i++) {
        int j = 1 + rand() % 9;
        int tmp = map[i];
        map[i] = map[j];
        map[j] = tmp;
    }
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            grid[r][c] = map[grid[r][c]];
}


// Function to remove cells from the grid to create the difficulty level of the puzzle
void remove_cells(int grid[9][9], int cells_to_remove) {
    int removed = 0;
    while (removed < cells_to_remove) {
        int r = rand() % 9;
        int c = rand() % 9;
        if (grid[r][c] != 0) {
            grid[r][c] = 0;
            removed++;
        }
    }
}

// Function to initialize the Sudoku game
void sudoku_init(SudokuGame *game, SudokuDifficulty difficulty) {
    memset(game, 0, sizeof(SudokuGame)); // Clear the game structure, set all values to 0
    game->difficulty = difficulty; // Set the difficulty level
    game->state = GAME_RUNNING; // Set initial game state to running
    game->start_time = time(NULL); // Record the start time
    game->elapsed_time = 0.0;   // Initialize elapsed time to 0, will be updated later in the game loop 
    game->selected_row = 0;     // Start with the first cell selected, top-left corner
    game->selected_col = 0;

    srand(time(NULL)); // Seed the random number generator

    int puzzle[9][9];   // Temporary puzzle grid
    memcpy(puzzle, solved_grid, sizeof(solved_grid)); // Start with a solved grid
    shuffle_rows(puzzle);   
    shuffle_cols(puzzle);
    permute_numbers(puzzle);
    memcpy (game->solution, puzzle, sizeof(puzzle)); // Save the solution before removing cells

    int cells_to_remove;    // Determine number of cells to remove based on difficulty
    if (difficulty == EASY) {
        cells_to_remove = 35;
    } else if (difficulty == MEDIUM) {
        cells_to_remove = 45;
    } else {
        cells_to_remove = 55;
    }
    remove_cells(puzzle, cells_to_remove);  // Remove cells to create the puzzle

    for (int r = 0; r < 9; r++) {   // Copy the puzzle into the game grid
        for (int c = 0; c < 9; c++) {   // Set cell value and fixed status
            int value = puzzle[r][c];   // 0 if empty
            game->grid.cells[r][c].value = value;   // Set cell value
            if (value != 0) {   // If the cell is part of the initial puzzle
                game->grid.cells[r][c].fixed = 1;   // Mark as fixed
            } else {    
                game->grid.cells[r][c].fixed = 0;   // Else, mark as not fixed
            }
        }
    }
}

// Returns 1 if board filled and valid, 0 otherwise
// Allows multiple valid solution
int sudoku_check_win(SudokuGame *game) {
    int row, col;
    int filled = 1;

    // Check for completeness and duplicates in rows/columns
    for (row = 0; row < SUDOKU_SIZE; row++) {
        int row_seen[10] = {0};
        int col_seen[10] = {0};

        for (col = 0; col < SUDOKU_SIZE; col++) {
            int val_row = game->grid.cells[row][col].value;
            int val_col = game->grid.cells[col][row].value;

            if (val_row == 0 || val_col == 0)
                filled = 0;

            if (val_row != 0) {
                if (row_seen[val_row])
                    goto invalid;
                row_seen[val_row] = 1;
            }
            if (val_col != 0) {
                if (col_seen[val_col])
                    goto invalid;
                col_seen[val_col] = 1;
            }
        }
    }

    // Check 3x3 boxes
    for (int box_row = 0; box_row < 3; box_row++) {
        for (int box_col = 0; box_col < 3; box_col++) {
            int seen[10] = {0};
            for (int r = 0; r < 3; r++) {
                for (int c = 0; c < 3; c++) {
                    int val = game->grid.cells[box_row * 3 + r][box_col * 3 + c].value;
                    if (val == 0)
                        filled = 0;
                    if (val != 0) {
                        if (seen[val])
                            goto invalid;
                        seen[val] = 1;
                    }
                }
            }
        }
    }

    if (!filled)
        return 0; // Still playing, not filled yet

    // If filled and valid
    game->state = GAME_WON;
    return 1;

invalid:
    if (filled)
        game->state = GAME_LOST; // Only set lost if board is fully filled
    return 0;
}


// For testing purposes, prints the Sudoku grid to console
void print_sudoku(SudokuGame *game) {
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            int v = game->grid.cells[r][c].value;
            if (v == 0)
                printf(". ");
            else
                printf("%d ", v);
            if ((c + 1) % 3 == 0 && c != 8) printf("| ");
        }
        printf("\n");
        if ((r + 1) % 3 == 0 && r != 8) printf("------+-------+------\n");
    }
}

