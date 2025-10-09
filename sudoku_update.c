#include "sudoku.h"
#include "dtekv_board.h"

// This function updates the game state based on input action, called from main loop
void sudoku_update(SudokuGame *game, InputAction action) {
    if (game->state != GAME_RUNNING) return;

    int row = game->selected_row;
    int col = game->selected_col;

    switch (action) {
        case INPUT_UP:
            if (row > 0) game->selected_row--;
            break;
        case INPUT_DOWN:
            if (row < SUDOKU_SIZE - 1) game->selected_row++;
            break;
        case INPUT_LEFT:
            if (col > 0) game->selected_col--;
            break;
        case INPUT_RIGHT:
            if (col < SUDOKU_SIZE - 1) game->selected_col++;
            break;
        case INPUT_INCREMENT:
            if (!game->grid.cells[row][col].fixed) {
                int value = game->grid.cells[row][col].value;
                value = (value % 9) + 1; // Cycle 1-9
                game->grid.cells[row][col].value = value;
            }
            break;
        case INPUT_ERASE:
            if (!game->grid.cells[row][col].fixed) {
                game->grid.cells[row][col].value = 0;
            }
            break;
        default:
            break;
    }
}
