#include <stdint.h> // For uint32_t which is used in run_sudoku to track state changes
#include "main_menu.h"
#include "minesweeper.h"
#include "dtekv_board.h"  // Uncomment when you have Sudoku header
#include "sudoku_input_vga.h"
#include "sudoku_puzzles.h"
#include "sudoku_vga.h"
#include"sudoku.h"

// VGA Memory Addresses extern because they are defined in sudoku_vga.c
extern volatile char *VGA;  
extern volatile int  *VGA_ctrl;
extern volatile int  *SWITCHES;
extern volatile int  *keys1;


int menu_state = MENU_STATE_MAIN;
int game_selection = MENU_MINEWEEPER;

//Initliazlised menu statment
void init_main_menu(void) {
    menu_state = MENU_STATE_MAIN;
    game_selection = MENU_MINEWEEPER;
}

void draw_main_menu(int selection) {
    // Clear screen with background color
    draw_rect(0, 0, 320, 240, light_blue);
    
    // Draw title
    char* title = "SELECT GAME";
    int title_x = 120;
    int title_y = 50;
    
    // Draw selection box
    int box_y = 100 + (selection * 60);
    draw_rect(80, box_y, 160, 40, yellow);
    
    // Draw Minesweeper option
    draw_rect(85, 105, 150, 30, game_selection == MENU_MINEWEEPER ? white : gray);
    // You could add text rendering here
    
    // Draw Sudoku option  
    draw_rect(85, 165, 150, 30, game_selection == MENU_SUDOKU ? white : gray);
    // You could add text rendering here
    
    // Draw simple text indicators (using blocks)
    // Minesweeper text
    int text_x = 100;
    int text_y = 115;
    for(int i = 0; i < 10; i++) {
        draw_block(text_x + i*8, text_y, 6, 8, black);
    }
    
    // Sudoku text
    text_y = 175;
    for(int i = 0; i < 6; i++) {
        draw_block(text_x + i*8, text_y, 6, 8, black);
    }
    
    // Draw instructions
    draw_rect(60, 220, 200, 15, dark_gray);
    // Instruction text could be added here

    *VGA_ctrl = 1; // Kick DMA to update screen
}

int handle_menu_input(void) {
    int current_switches = *SWITCHES;
    int current_keys     = *keys1;

    static int last_switches;
    static int last_keys;
    static int seeded = 0;

    /* seed previous state on first call to avoid a false/missed edge */
    if (!seeded) {
        last_switches = current_switches;
        last_keys     = current_keys;
        seeded        = 1;
    }

    /* SW0 selects which game is highlighted */
    if (current_switches & (1 << SW_SELECT_GAME)) {
        game_selection = MENU_SUDOKU;
    } else {
        game_selection = MENU_MINEWEEPER;
    }

    /* KEY1 (KEY_enter) is active-low: pressed = 0, released = 1 */
    {
        int prev_bit = (last_keys    >> KEY_enter) & 1;
        int curr_bit = (current_keys >> KEY_enter) & 1;
        int press_edge = (prev_bit == 1) && (curr_bit == 0);

        if (press_edge) {
            last_switches = current_switches;
            last_keys     = current_keys;

            if (game_selection == MENU_MINEWEEPER) {
                return MENU_STATE_MINEWEEPER;
            } else {
                return MENU_STATE_SUDOKU;
            }
        }
    }

    last_switches = current_switches;
    last_keys     = current_keys;

    return MENU_STATE_MAIN;
}


void run_minesweeper(void) {
    // Initialize and run minesweeper game
    init_game(0);  // Start with easy difficulty
    draw_board();
    
    while(1) {
        handle_input();
        
        // Redraw on state change
        static int last_state = 0;
        int current_state = game.cursor_x | (game.cursor_y << 8) |
                           (game.game_over << 16) | (game.game_won << 17);
        
        if(current_state != last_state) {
            draw_board();
            if(game.game_over || game.game_won) {
                draw_gameover();
            }
            last_state = current_state;
        }
        
        delay(1);
    }
}

SudokuDifficulty get_selected_difficulty(void) {
    volatile int *SWITCHES = (volatile int *) SWITCH_base;
    int switches = *SWITCHES;

    // Use the same bits your Sudoku main used (typically SW1..SW3)
    if (switches & (1 << 3)) return HARD;    // SW3
    if (switches & (1 << 2)) return MEDIUM;  // SW2
    if (switches & (1 << 1)) return EASY;    // SW1
    return EASY; // default
}

void run_sudoku(void) {
    SudokuGame game;

    // Choose difficulty by switches, KEY1 to confirm
    volatile int *keys = (volatile int *) KEY1_base;
    const int KEY_MASK = (1 << KEY_enter);  // KEY1 bit from dtekv_board.h
    int prev_keys = *keys;                  // init edge detector

    for (;;) {  // Difficulty selection loop
        int curr = *keys;
        int was_pressed = !(prev_keys & KEY_MASK);  // active-low
        int is_pressed  = !(curr      & KEY_MASK);
        int key1_press_edge = (!was_pressed && is_pressed);

        if (key1_press_edge) {
            break; // latch difficulty on the exact press
        }
        prev_keys = curr;
        delay(1);
    }

    SudokuDifficulty difficulty = get_selected_difficulty();
    sudoku_init(&game, difficulty);  // init FIRST

    // Initial draw
    sudoku_render_vga(&game);

    // Track visible state to avoid unnecessary redraws
    uint32_t last_sig =
        (uint32_t)game.selected_col |
        ((uint32_t)game.selected_row << 8) |
        ((uint32_t)game.state        << 16);

    // --- Game loop ---
    for (;;) {
        // Read one action per press (your get_input_vga already does edge detection)
        InputAction action = get_input_vga();

        if (action == INPUT_EXIT) {
            // If you ever route an EXIT action (e.g., SW7+KEY), go back to menu
            menu_state = MENU_STATE_MAIN;
            return;
        }

        if (action != INPUT_NONE) {
            sudoku_update(&game, action);
            sudoku_check_win(&game);
        }

        // Redraw only when something visible changed
        uint32_t sig =
            (uint32_t)game.selected_col |
            ((uint32_t)game.selected_row << 8) |
            ((uint32_t)game.state        << 16);

        if (sig != last_sig) {
            sudoku_render_vga(&game);
            last_sig = sig;
        }

        // If finished, wait for a KEY1 press edge to return to menu
        int curr = *keys;
        int was_pressed = !(prev_keys & KEY_MASK);
        int is_pressed  = !(curr      & KEY_MASK);
        int key1_press_edge = (!was_pressed && is_pressed);

        if ((game.state == GAME_WON || game.state == GAME_LOST) && key1_press_edge) {
            menu_state = MENU_STATE_MAIN;
            return;
        }
        prev_keys = curr;

        delay(1);
    }
}
