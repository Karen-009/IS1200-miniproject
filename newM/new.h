#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include "dtekv_board.h"

// Game constants
#define GRID_SIZE 16
#define CELL_SIZE 14
#define GRID_OFFSET_X 8
#define GRID_OFFSET_Y 8

// Game states
#define STATE_MENU 0
#define STATE_PLAYING 1
#define STATE_WON 2
#define STATE_LOST 3

// Cell states
#define CELL_HIDDEN 0
#define CELL_REVEALED 1
#define CELL_FLAGGED 2
#define CELL_MINE 4
#define CELL_EXPLODED 8

// Difficulty levels
#define DIFFICULTY_EASY 0
#define DIFFICULTY_MEDIUM 1
#define DIFFICULTY_HARD 2

// Game structure
typedef struct {
    int grid[GRID_SIZE][GRID_SIZE];
    int mine_count;
    int flags_placed;
    int cells_revealed;
    int cursor_x, cursor_y;
    int game_state;
    int difficulty;
    int move_count;
    int last_move_x, last_move_y;
    int move_pending;
    unsigned int random_seed;
} MinesweeperGame;

// Function prototypes
void init_game(MinesweeperGame* game, int difficulty);
void place_mines(MinesweeperGame* game);
int count_adjacent_mines(MinesweeperGame* game, int x, int y);
void reveal_cell(MinesweeperGame* game, int x, int y);
void toggle_flag(MinesweeperGame* game, int x, int y);
void draw_game(MinesweeperGame* game);
void draw_cell(MinesweeperGame* game, int x, int y);
void draw_cursor(MinesweeperGame* game);
void draw_ui(MinesweeperGame* game);
void handle_input(MinesweeperGame* game);
int check_win_condition(MinesweeperGame* game);
void reset_game(MinesweeperGame* game);
void handle_interrupt();

// Custom random number generator (Xorshift)
unsigned int custom_rand(MinesweeperGame* game);
void custom_srand(MinesweeperGame* game, unsigned int seed);

#endif