// Karen, minesweeper header file

#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <stdint.h>
#include "sudoku.h"

/* Game constants */
#define CELL_SIZE 12
#define GRID_MAX_COLS 26   /* = 320 / 12 */
#define GRID_MAX_ROWS 20   /* = 240 / 12 */
#define MAX_CELLS (GRID_MAX_ROWS * GRID_MAX_COLS)

/* Cell states */
typedef enum { HIDDEN=0, REVEALED=1, FLAGGED=2 } CellState;

/* Level specification */
typedef struct {
    int cols, rows;
    int mines;
} LevelSpec;

// External declarations for game state
extern uint8_t mine_grid[GRID_MAX_ROWS][GRID_MAX_COLS];
extern uint8_t adj[GRID_MAX_ROWS][GRID_MAX_COLS];
extern uint8_t state_grid[GRID_MAX_ROWS][GRID_MAX_COLS];
extern int g_rows, g_cols, g_mines;
extern int revealed_count;
extern int game_over;
extern int cursor_r, cursor_c;

// Function declarations

int minesweeper(void);
int abs(int n);
void draw_text(int x, int y, const char *text, uint8_t color);

// Game initialization
void start_new_game(SudokuDifficulty d);
void clear_board_state(void);
void place_mines(int rows, int cols, int mines, int safe_r, int safe_c);
void compute_adj(int rows, int cols);

// Game logics
void reveal_cell(int r, int c);
void toggle_flag(int r, int c);
void flood_reveal(int sr, int sc);

// Rendering
void render_board(void);
void draw_cell_border(int r, int c, uint8_t border_color);
void draw_digit_in_cell(int grid_r, int grid_c, int digit, uint8_t color);
void draw_text(int x, int y, const char *text, uint8_t color);


// Utility functions
uint32_t rand32(void);
void busy_wait(volatile int n);
void put_pixel(int x, int y, uint8_t color);
void fill_rect(int x0, int y0, int w, int h, uint8_t color);

// Input reading
uint32_t read_switches(void);
uint32_t read_keys(void);
void wait_key_release_all(void);

#endif