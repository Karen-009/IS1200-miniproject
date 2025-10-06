#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "minesweeper.h"
// #include "sudoku.h"

// Menu states
#define MENU_STATE_MAIN 0
#define MENU_STATE_MINEWEEPER 1
#define MENU_STATE_SUDOKU 2

// Menu selection
#define MENU_MINEWEEPER 0
#define MENU_SUDOKU 1

// Switch assignments for menu
#define SW_SELECT_GAME 0  // Use switch 0 to select between games
#define KEY_enter 0      // Use KEY0 to confirm selection

// Function declarations
void init_main_menu(void);
void draw_main_menu(int selection);
int handle_menu_input(void);
void run_minesweeper(void);
void run_sudoku(void);

// Global variables
extern int menu_state;
extern int game_selection;

#endif