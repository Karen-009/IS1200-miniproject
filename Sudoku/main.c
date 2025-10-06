#include "dtekv_board.h"
#include "sudoku.h"
#include "sudoku_vga.h"
#include "sudoku_input_vga.h"
#include <stdint.h>

// small delay
static inline void delay(int cycles) {
    volatile int i = 0;
    while (i < cycles * 10000) { i++; }
}


// Get selected difficulty based on switch positions (Switch 1–3)
static inline SudokuDifficulty get_selected_difficulty(void) {
    volatile int *SWITCHES = (volatile int *) SWITCH_base;
    int switches = *SWITCHES;

    if (switches & (1 << 3)) return HARD;    // SW3
    if (switches & (1 << 2)) return MEDIUM;  // SW2
    if (switches & (1 << 1)) return EASY;    // SW1
    return EASY; // default
}


int main(void) {
    SudokuGame game;
    SudokuDifficulty difficulty = EASY;

    volatile int *keys1 = (volatile int *) KEY1_base;
    const int KEY_MASK = (1 << KEY_enter);

    // init edge detector with current raw value (active-low)
    int prev_keys = *keys1;

    // difficulty selection loop
    while (1) {
        int current_keys = *keys1;

        // active-low: pressed when bit == 0
        int was_pressed = !(prev_keys   & KEY_MASK);
        int is_pressed  = !(current_keys & KEY_MASK);
        int key_enter_press_edge = (!was_pressed && is_pressed);

        if (key_enter_press_edge) {
            // latch difficulty exactly when button is pressed
            difficulty = get_selected_difficulty();
            prev_keys = current_keys;
            break;
        }

        prev_keys = current_keys;
        delay(1);
    }

    // Initialize game AFTER difficulty is chosen
    sudoku_init(&game, difficulty);

    // Initial draw
    sudoku_render_vga(&game);

    // Track last drawn state (use selected_col/selected_row as in sudoku_vga.c)
    uint32_t last_sig =
        (uint32_t)game.selected_col |
        ((uint32_t)game.selected_row << 8) |
        ((uint32_t)game.state        << 16);

    // main game loop
    while (1) {
        int need_redraw = 0;

        // outputs only on KEY press edges (active-low fixed)
        InputAction action = get_input_vga();

        if (action == INPUT_EXIT) {
            break; // back to difficulty select
        }

        if (action != INPUT_NONE) {
            sudoku_update(&game, action);
            sudoku_check_win(&game);
            need_redraw = 1;
        }

        // If finished, wait for a single KEY press edge to exit
        int current_keys = *keys1;
        int was_pressed  = !(prev_keys   & KEY_MASK);
        int is_pressed   = !(current_keys & KEY_MASK);
        int key_press_edge = (!was_pressed && is_pressed);

        if ((game.state == GAME_WON || game.state == GAME_LOST) && key_press_edge) {
            prev_keys = current_keys;
            break;
        }
        prev_keys = current_keys;

        // Redraw only if something visible changed
        uint32_t sig =
            (uint32_t)game.selected_col |
            ((uint32_t)game.selected_row << 8) |
            ((uint32_t)game.state        << 16);

        if (sig != last_sig) {
            need_redraw = 1;
            last_sig = sig;
        }

        if (need_redraw) {
            sudoku_render_vga(&game);
        }

        delay(1);
    }

    return 0;
}
