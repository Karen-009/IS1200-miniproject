# include <stdio.h>
<<<<<<< HEAD
#include <time.h>
=======
# include <stdlib.h>
# include <math.h>
# include <time.h>
# include <stdbool.h>
# include "system.h"
>>>>>>> 59dfb70 (Added some general structs)

typedef enum {
    EASY = 0,
    MEDIUM = 1,
    HARD = 2,
    VERY_HARD = 3,
    EXPERT = 4
} Difficulty;

typedef struct { // Cell structure to represent each cell in the grid, has to be initialized later
    bool mine;
    int adjacent_mines; // Number of adjacent mines
    int revealed; // 0 = not revealed, 1 = revealed, 2 = flagged
    int flagged; // 0 = not flagged, 1 = flagged
} Cell;

typedef struct { // Grid structure to represent the entire grid, has to be initialized later
    Cell *cells; // Pointer to create a 2D array of Cell structures
    int rows;
    int cols;
} Grid;

const struct {
    int rows, cols, mines;
} difficulty_settings[] = {
    {8, 8, 10},      // EASY
    {12, 12, 20},    // MEDIUM
    {16, 16, 30},    // HARD
    {20, 20, 40},    // VERY_HARD
    {25, 25, 60}    // EXPERT
};

Grid game_grid; // Global variable to hold the game grid


