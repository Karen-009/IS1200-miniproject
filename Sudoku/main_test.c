#include "sudoku.h"
#include <stdio.h>

int main() {
    SudokuGame game;
    sudoku_init(&game, EASY); // or MEDIUM/HARD

    while (1) {
        print_sudoku(&game);

        int row, col, val;
        printf("Enter row col value (1-9 1-9 1-9): ");
        scanf("%d %d %d", &row, &col, &val);
        row--; col--;

        if (!game.grid.cells[row][col].fixed)
            game.grid.cells[row][col].value = val;

        if (sudoku_check_win(&game)) {
            printf("You win!\n");
            break;
        }
        if (game.state == GAME_LOST) {
            printf("Board is invalid. You lost!\n");
            break;
        }
    }
    return 0;
}