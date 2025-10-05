//header file for sudoku.c
#ifndef SUDOKU_H    //  correct and prevents multiple inclusions
#define SUDOKU_H    // SUDOKU_H, a unique identifier for this header file
#include <time.h> // For time tracking
#define SUDOKU_SIZE 9  // Define the size of the Sudoku grid

typedef struct {
    int value; // Value of the cell (0 if empty)
    int fixed; // 1 if the cell is part of the initial puzzle, 0 otherwise
} SudokuCell;

typedef struct {
    SudokuCell cells[SUDOKU_SIZE][SUDOKU_SIZE]; // 9x9 grid of Sudoku cells
} SudokuGrid;

typedef enum {  // Enum for difficulty levels
    EASY = 0,
    MEDIUM = 1,
    HARD = 2
} SudokuDifficulty;

typedef enum {  // Enum for game states
    GAME_RUNNING,
    GAME_WON,
    GAME_LOST
} GameState;

typedef enum {  // Enum for input actions
    INPUT_NONE, 
    INPUT_UP, 
    INPUT_DOWN, 
    INPUT_LEFT, 
    INPUT_RIGHT, 
    INPUT_INCREMENT, // Increment cell value (SW9)
    INPUT_ERASE,     // Erase cell value (SW8)
    INPUT_ENTER,   
    INPUT_EXIT,
} InputAction; 


typedef struct {
    SudokuGrid grid; // The Sudoku grid
    int solution[SUDOKU_SIZE][SUDOKU_SIZE]; // The solution grid for validation
    int selected_row; // Currently selected row
    int selected_col; // Currently selected column
    time_t start_time; // Time when the game started
    time_t stop_time; // Time when the game stopped
    GameState state; // Current game state
    double elapsed_time; // Time elapsed since the start of the game
    int difficulty; // Difficulty level, 0 = easy, 1 = medium, 2 = hard
    int show_timer; // 1 if timer is shown on VGA, 0 otherwise
    int score; // Calculate score to show on VGA 
} SudokuGame;

// Logic functions
void sudoku_init(SudokuGame *game, SudokuDifficulty difficulty); 
void sudoku_update(SudokuGame *game, InputAction action);
int sudoku_check_win(SudokuGame *game);
void sudoku_play(void);

// VGA rendering function & input fot DTEKV board
void sudoku_render_vga(const SudokuGame *game); // Draw grid, numbers, selection, timer, score on VGA 
InputAction get_input_vga(void);    // Read switches and keys and map to InputAction

// Debugging function to print the Sudoku grid to console
void print_sudoku(SudokuGame *game);


#endif 