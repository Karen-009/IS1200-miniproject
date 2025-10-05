#include "dtekv_board.h"
#include "sudoku.h"
#include "sudoku_vga.h"
#include "sudoku_input_vga.h"

// Get selected difficulty based on switch positions 
SudokuDifficulty get_selected_difficulty(void) {
    volatile int *SWITCHES = (volatile int *) SWITCH_base;
    int switches = *SWITCHES;

    if (switches & (1 << SW_l3)) return HARD;
    if (switches & (1 << SW_l2)) return MEDIUM;
    if (switches & (1 << SW_l1)) return EASY;
    return EASY; // Default to EASY if no switch is set
}

int main(void) {
    SudokuGame game;

    SudokuDifficulty difficulty = get_selected_difficulty();
    while(1) {
        difficulty = get_selected_difficulty();
        sudoku_render_vga(&game); // Initial render before starting
        volatile int *keys1 = (volatile int *) KEY1_base; //address of KEY1
        if (*keys1 & 0x1)   // If KEY0 is pressed, start the game
            break; // Exit loop to start game
    }

    sudoku_init(&game, difficulty); // Initialize game with selected difficulty

    while (1) {
        sudoku_render_vga(&game); // Draw current game state

        if (game.state == GAME_WON || game.state == GAME_LOST) {
                // Wait for KEY1 to restart (and go back to difficulty select)
                volatile int *keys1 = (volatile int *)KEY1_base;
                while (!(*keys1 & 0x1)) {
                    sudoku_render_vga(&game);
                }
                while (*keys1 & 0x1); // Debounce
                break; // Exit loop to select difficulty again
            }

            InputAction action = get_input_vga();
            if (action != INPUT_NONE) {
                sudoku_update(&game, action);
                sudoku_check_win(&game);
        }
    }
    return 0;
}
