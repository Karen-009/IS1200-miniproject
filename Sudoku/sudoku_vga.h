#ifndef SUDOKU_VGA_H
#define SUDOKU_VGA_H
#include "sudoku.h"

void sudoku_render_vga(const SudokuGame *game);

// Pixel drawing 
void draw_pixel(int x, int y, char color);
void draw_rect(int x, int y, int width, int height, char color);
void draw_block(int x, int y, int width, int height, char color);

// Digit/cell drawing
void draw_digit(int grid_x, int grid_y, int number);
void draw_sudoku_cell(int cell_x, int cell_y, const SudokuGame *game);

// Board and grid drawing
void draw_sudoku_board(const SudokuGame *game);
void draw_bold_grid_lines(void);    // Draw bold lines for 3x3 boxes

// Cursor and highlights
void draw_cursor(const SudokuGame *game);

// Timer 
void draw_timer(const SudokuGame *game);

// End game screens 
void draw_game_over(const SudokuGame *game);
void draw_game_won(const SudokuGame *game);

// Input and cursor movement (to be called from main loop)
void handle_digit_entry(SudokuGame *game); 
void handle_erase(SudokuGame *game); 
void move_cursor(SudokuGame *game, int dx, int dy);

// Background color for 3x3 boxes
char get_box_color(int box_row, int box_col);

#endif
