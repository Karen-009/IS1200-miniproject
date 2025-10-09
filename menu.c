#include <stdint.h> // For uint32_t which is used in run_sudoku to track state changes
#include "main_menu.h"
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

// Map ASCII letters to font array index 0-25 (A-Z)
int font_index(char c) {
    if (c >= 'A' && c <= 'Z') 
        return c - 'A';
    return 0; // Default to 'A' for unsupported characters
}

void draw_char(int x, int y, char c, unsigned char color) {
    if (c < 'A' || c > 'Z') return; // Unsupported character

    int index = font_index(c);
    for (int row = 0; row < 8; row++) {
        unsigned char row_data = font8x8_AZ[index][row];
        for (int col = 0; col < 8; ++col) {
            if (row_data & (1 << (7 - col))) { // Check if the bit is set
                draw_pixel(x + col, y + row, color);
            }
        }
    }
}

// Draw a text string at (x, y)
void draw_text(int x, int y, const char *text, unsigned char color) {
    while (*text) {
        if (*text == ' ') {
            x += 8; // Space width
        } else {
            draw_char(x, y, *text, color);
            x += 8;
        }
        text++;
    }
}

//Initliazlised menu statment
void init_main_menu(void) {
    menu_state = MENU_STATE_MAIN;
    game_selection = MENU_MINEWEEPER;
}

// Draw the main menu with current selection highlighted
void draw_main_menu(int selection) {
    // Clear screen with background color
    draw_rect(0, 0, 320, 240, light_blue);

    
    // Draw selection box
    int box_y = 100 + (selection * 60);
    draw_rect(80, box_y, 160, 40, yellow);
    
    // Draw Minesweeper option
    draw_rect(85, 105, 150, 30, game_selection == MENU_MINEWEEPER ? white : gray);
    // You could add text rendering here
    
    // Draw Sudoku option  
    draw_rect(85, 165, 150, 30, game_selection == MENU_SUDOKU ? white : gray);
    // You could add text rendering here
    
    // Draw title text
    draw_text(100, 115, "MINESWEEPER", black);
    draw_text(130, 175, "SUDOKU", black);
    
    // Draw instructions
    draw_text(116, 20, "SELECT GAME", pink);    // centered at top

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
    minesweeper();
}

void run_sudoku(void) {
    SudokuGame game;

    // Entropy collection for RNG seed
    volatile int *keys = (volatile int *) KEY1_base;
    volatile int *SWITCHES = (volatile int *) SWITCH_base;
    const int KEY_MASK = (1 << KEY_enter);  // KEY1 bit from dtekv_board.h
    int prev_keys = *keys;                  // init edge detector
    unsigned seed = 0x6D2B79F5u;

    while (1) {
        int curr = *keys;

        // Entropy for RNG
        seed ^= (seed << 13);
        seed ^= (seed >> 17);
        seed ^= (seed << 5);
        seed ^= (unsigned)curr;
        seed ^= ((unsigned)SWITCHES << 16);

        // KEY1 edge detection (active-low)
        int was_pressed = !(prev_keys & KEY_MASK);
        int is_pressed  = !(curr      & KEY_MASK);
        int key1_press_edge = (!was_pressed && is_pressed);

        if (key1_press_edge) {
            prev_keys = curr;
            break; // Confirm and continue
        }
        prev_keys = curr;
        delay(5); // Small delay for debounce and flicker prevention
    }

    // Seed RNG here so each Sudoku launch is fresh/random
    srand(seed);

    // Difficulty selection
    SudokuDifficulty difficulty = get_selected_difficulty_from_switches();
    sudoku_init(&game, difficulty);

    // Initial draw
    sudoku_render_vga(&game);

    // Track visible state to avoid unnecessary redraws
    uint32_t last_sig =
        (uint32_t)game.selected_col |
        ((uint32_t)game.selected_row << 8) |
        ((uint32_t)game.state        << 16);

    // Game loop
    for (;;) {
    InputAction action = get_input_vga();

    int needs_redraw = 0;

    if (action == INPUT_EXIT) {
        menu_state = MENU_STATE_MAIN;
        return;
    }

    if (action != INPUT_NONE) {
        sudoku_update(&game, action);
        needs_redraw = 1;
    }

    // Only check for win/loss after the user submits (KEY1) and board is full
    if (sudoku_is_full(&game) && game.state == GAME_RUNNING) {
        // Show "Press KEY1 to check" message (if you want)
        sudoku_render_vga(&game); // draws the full board and (optionally) "Press KEY1 to check"

        // Wait for KEY1 press
        int prev_keys = *keys;
        while (1) {
            int curr_keys = *keys;
            int was_pressed = !(prev_keys & KEY_MASK);
            int is_pressed  = !(curr_keys & KEY_MASK);
            int key1_press_edge = (!was_pressed && is_pressed);

            if (key1_press_edge) {
                sudoku_check_win(&game); // Now check for win/loss
                needs_redraw = 1;
                break;
            }
            prev_keys = curr_keys;
            delay(1);
        }
    }

    if (needs_redraw) {
        sudoku_render_vga(&game);
    }

    // Wait for KEY1 to return to menu after win/loss
    if ((game.state == GAME_WON || game.state == GAME_LOST)) {
        int prev_keys = *keys;
        while (1) {
            int curr_keys = *keys;
            int was_pressed = !(prev_keys & KEY_MASK);
            int is_pressed  = !(curr_keys & KEY_MASK);
            int key1_press_edge = (!was_pressed && is_pressed);

            if (key1_press_edge) {
                menu_state = MENU_STATE_MAIN;
                return;
            }
            prev_keys = curr_keys;
            delay(1);
        }
    }

    delay(1);
}
}

void delay(int ms){
    volatile int i, j;
    for (i = 0; i < ms * 10000; i++)
    {
        j = i;
    }
}

void test(void) {
}

// Add this function to menu.c
SudokuDifficulty get_selected_difficulty_from_switches(void) {
    volatile int *SWITCHES = (volatile int *) SWITCH_base;
    int switches = *SWITCHES;
    
    if (switches & (1 << SW_l3)) return HARD;    // SW3
    if (switches & (1 << SW_l2)) return MEDIUM;  // SW2  
    if (switches & (1 << SW_l1)) return EASY;    // SW1
    return EASY; // default
}