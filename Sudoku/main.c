/*
#include "dtekv_board.h"
#include "sudoku.h"
#include "sudoku_vga.h"
#include "sudoku_input_vga.h"
#include <stdint.h>
#include <stdlib.h>   // for srand()

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

    volatile int *keys1    = (volatile int *) KEY1_base;
    volatile int *SWITCHES = (volatile int *) SWITCH_base;
    const int KEY_MASK = (1 << KEY_enter);

    // init edge detector with current raw value (active-low)
    int prev_keys = *keys1;

    // collect entropy while waiting for KEY1 press, then seed once
    unsigned seed = 0x6D2B79F5u;

    // difficulty selection loop (and entropy gather)
    while (1) {
        int current_keys = *keys1;

        // simple xorshift mixing each iteration + live inputs
        seed ^= (seed << 13);
        seed ^= (seed >> 17);
        seed ^= (seed << 5);
        seed ^= (unsigned)current_keys;
        seed ^= ((unsigned)(*SWITCHES) << 16);

        // active-low: pressed when bit == 0
        int was_pressed = !(prev_keys    & KEY_MASK);
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

    // seed RNG once here (sudoku_init must NOT reseed)
    srand(seed);

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
        int was_pressed  = !(prev_keys    & KEY_MASK);
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
*/

#include "dtekv_board.h"
#include "sudoku.h"
#include "sudoku_vga.h"
#include "sudoku_input_vga.h"
#include <stdint.h>
#include <stdlib.h>   // srand, rand

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

    volatile int *keys1    = (volatile int *) KEY1_base;
    volatile int *SWITCHES = (volatile int *) SWITCH_base;
    const int KEY_MASK = (1 << KEY_enter);   // KEY1 bit index from dtekv_board.h (active-low)

    // init edge detector with current raw value (active-low)
    int prev_keys = *keys1;

    // collect entropy while waiting for KEY1 press, then seed once
    unsigned seed = 0x6D2B79F5u;

    // difficulty selection loop (and entropy gather)
    while (1) {
        int current_keys = *keys1;

        // simple xorshift mixing each iteration + live inputs
        seed ^= (seed << 13);
        seed ^= (seed >> 17);
        seed ^= (seed << 5);
        seed ^= (unsigned)current_keys;
        seed ^= ((unsigned)(*SWITCHES) << 16);

        // active-low: pressed when bit == 0
        int was_pressed = !(prev_keys    & KEY_MASK);
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

    // seed RNG once here (sudoku_init must NOT reseed)
    srand(seed);

    // Initialize game AFTER difficulty is chosen
    sudoku_init(&game, difficulty);

    // Initial draw
    sudoku_render_vga(&game);

    // End screen control
    int end_mode = 0;      // 0=playing, 1=ended (won/lost)
    int wait_release = 0;  // after end, wait for a release before accepting a new press

    // main game loop
    while (1) {
        // If in end mode, show end screen and handle exit via "release -> new press"
        if (end_mode) {
            int curr = *keys1;

            // active-low release check
            if (wait_release) {
                if ((curr & KEY_MASK) != 0) {
                    wait_release = 0;  // released; now wait for next press
                }
            } else {
                int was = !(prev_keys & KEY_MASK);
                int is  = !(curr      & KEY_MASK);
                int press_edge = (!was && is);
                if (press_edge) {
                    break; // leave Sudoku (back to menu/difficulty select)
                }
            }

            prev_keys = curr;
            // End screen is already drawn by sudoku_render_vga() when state != GAME_RUNNING
            delay(1);
            continue;
        }

        // Normal gameplay
        InputAction action = get_input_vga();

        if (action == INPUT_EXIT) {
            break; // optional early exit if you wire one
        }

        if (action != INPUT_NONE) {
            // Apply the move
            sudoku_update(&game, action);
            // Validate state after the move
            sudoku_check_win(&game);
            // Render immediately so the last digit + overlay are visible this frame
            sudoku_render_vga(&game);

            // If the move finished the game, enter end mode and require a release first
            if (game.state == GAME_WON || game.state == GAME_LOST) {
                end_mode = 1;
                wait_release = 1;  // consume the finishing press; require release
            }
        }

        // Update key history last
        prev_keys = *keys1;

        delay(1);
    }

    return 0;
}
