#include "dtekv_board.h"
#include "sudoku.h"
#include "sudoku_vga.h"
#include "sudoku_input_vga.h"

// Get selected difficulty based on switch positions (Switch 1–3)
SudokuDifficulty get_selected_difficulty(void) {
    volatile int *SWITCHES = (volatile int *) SWITCH_base;
    int switches = *SWITCHES;

    if (switches & (1 << 3)) return HARD;    // Switch 3
    if (switches & (1 << 2)) return MEDIUM;  // Switch 2
    if (switches & (1 << 1)) return EASY;    // Switch 1
    return EASY; // Default if no switch is set
}

int main(void) {
    SudokuGame game;

    while (1) {
        // --- Difficulty selection loop ---
        SudokuDifficulty difficulty = get_selected_difficulty();
        sudoku_render_vga(&game); // Show initial empty board with difficulty info

        volatile int *keys1 = (volatile int *) KEY1_base;
        // Wait until KEY1 (confirm) is pressed to start
        if (*keys1 & (1 << KEY_enter)) {
            // Debounce KEY1
            while (*keys1 & (1 << KEY_enter));
            break; // Start the game
        }
    }

    // Initialize the game with selected difficulty
    sudoku_init(&game, get_selected_difficulty());

    // --- Main game loop ---
    while (1) {
        sudoku_render_vga(&game); // Draw current game state

        // Get input from switches + KEY1
        InputAction action = get_input_vga();

        // Exit to menu if exit key is pressed
        if (action == INPUT_EXIT) {
            break; // Return to main menu
        }

        if (action != INPUT_NONE) {
            sudoku_update(&game, action);  // Apply player action
            sudoku_check_win(&game);       // Check if game is won
        }

        // If game is finished, wait for KEY1 to restart
        if (game.state == GAME_WON || game.state == GAME_LOST) {
            while (!(*((volatile int *)KEY1_base) & (1 << KEY_enter))) {
                sudoku_render_vga(&game); // Keep showing final state
            }
            while (*((volatile int *)KEY1_base) & (1 << KEY_enter)); // Debounce
            break; // Exit to difficulty select
        }
    }

    return 0;
}