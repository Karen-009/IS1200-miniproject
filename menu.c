// Zunjee, Karen, menu implementation file for sudoku and minesweeper
#include <stdint.h> // For uint32_t which is used in run_sudoku to track state changes
#include "main_menu.h"
#include "dtekv_board.h"
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
    return 0; 
}

void draw_char(int x, int y, char c, unsigned char color) {    // Draw a single character at (x, y)
    if (c < 'A' || c > 'Z') return;     

    int index = font_index(c);     // Get index in font array
    for (int row = 0; row < 8; row++) {   // Each character is 8 pixels tall
        unsigned char row_data = font8x8_AZ[index][row];    // Get bitmap for this row
        for (int col = 0; col < 8; ++col) {     // Each character is 8 pixels wide
            if (row_data & (1 << (7 - col))) { // Check if the bit is set
                draw_pixel(x + col, y + row, color);    // Draw pixel if bit is 1
            }
        }
    }
}

// Draw a text string at (x, y)
void draw_text(int x, int y, const char *text, unsigned char color) {   // Draw text string starting at (x, y)
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
    
    // Draw Sudoku option  
    draw_rect(85, 165, 150, 30, game_selection == MENU_SUDOKU ? white : gray);
    
    // Draw title text
    draw_text(100, 115, "MINESWEEPER", black);
    draw_text(130, 175, "SUDOKU", black);
    
    // Draw select game instruction
    draw_text(116, 20, "SELECT GAME", pink); 

    *VGA_ctrl = 1; // Kick DMA to update screen
}

int handle_menu_input(void) {
    int current_switches = *SWITCHES;
    int current_keys     = *keys1;

    static int last_switches;   // For edge detection
    static int last_keys;      
    static int seeded = 0;

    // Seed previous state on first call to avoid a false/missed edge
    if (!seeded) {
        last_switches = current_switches;
        last_keys     = current_keys;
        seeded        = 1;
    }

    // SW0 selects which game is highlighted
    if (current_switches & (1 << SW_SELECT_GAME)) {
        game_selection = MENU_SUDOKU;   // SW0 = 1 selects Sudoku
    } else {
        game_selection = MENU_MINEWEEPER;   // SW0 = 0 selects Minesweeper
    }

    // KEY1 (KEY_enter) is active-low: pressed = 0, released = 1 
    {
        int prev_bit = (last_keys    >> KEY_enter) & 1;
        int curr_bit = (current_keys >> KEY_enter) & 1;
        int press_edge = (prev_bit == 1) && (curr_bit == 0);

        if (press_edge) {      // On KEY1 press
            last_switches = current_switches;   // Update previous state in order to avoid multi-fire
            last_keys     = current_keys;       

            if (game_selection == MENU_MINEWEEPER) {
                return MENU_STATE_MINEWEEPER;   // Start minesweeper
            } else {
                return MENU_STATE_SUDOKU;       // Start sudoku
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

    volatile int *keys = (volatile int *) KEY1_base;    
    volatile int *SWITCHES = (volatile int *) SWITCH_base;  
    const int KEY_MASK = (1 << KEY_enter);  // KEY1 mask, active-low
    int prev_keys = *keys;                  
    unsigned seed = 0x6D2B79F5u;    // Initial arbitrary seed for RNG, will be mixed with entropy

    while (1) { // Wait for KEY1 press to start
        int curr = *keys;   // Current key state

        // Entropy for RNG seeding, ensures different puzzle each time 
        seed ^= (seed << 13);   
        seed ^= (seed >> 17);
        seed ^= (seed << 5);
        seed ^= (unsigned)curr;
        seed ^= ((unsigned)SWITCHES << 16); 

        // KEY1 edge detection (active-low)
        int was_pressed = !(prev_keys & KEY_MASK);  // Previously pressed
        int is_pressed  = !(curr      & KEY_MASK);  // Currently pressed
        int key1_press_edge = (!was_pressed && is_pressed); // Rising edge detection

        if (key1_press_edge) {      // On KEY1 press
            prev_keys = curr;       // Update previous state
            break; 
        }
        prev_keys = curr;   // Update previous state
        delay(5);     
    }

    // Seed RNG here so each Sudoku game is random 
    srand(seed); 

    // Difficulty selection
    SudokuDifficulty difficulty = get_selected_difficulty_from_switches();  // Read switches to get difficulty
    sudoku_init(&game, difficulty); // Initialize game state

    // Initial draw
    sudoku_render_vga(&game);

    // Track visible state to avoid unnecessary redraws, only redraw on changes 
    uint32_t last_sig = // Pack key state and game state into a single integer, used to detect changes
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
        sudoku_render_vga(&game); // Redraw to show full board

        // Wait for KEY1 press
        int prev_keys = *keys;
        while (1) { 
            int curr_keys = *keys;
            int was_pressed = !(prev_keys & KEY_MASK);      // Previously pressed
            int is_pressed  = !(curr_keys & KEY_MASK);      // Currently pressed
            int key1_press_edge = (!was_pressed && is_pressed); // Rising edge detection

            if (key1_press_edge) {  // On KEY1 press
                sudoku_check_win(&game); // Now check for win/loss
                needs_redraw = 1;       // Redraw to show win/loss state
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

void delay(int ms){ // Simple busy-wait delay
    volatile int i, j;
    for (i = 0; i < ms * 10000; i++)
    {
        j = i;
    }
}

void test(void) { 
}

SudokuDifficulty get_selected_difficulty_from_switches(void) {
    volatile int *SWITCHES = (volatile int *) SWITCH_base;
    int switches = *SWITCHES;
    
    if (switches & (1 << SW_l3)) return HARD;    // SW3
    if (switches & (1 << SW_l2)) return MEDIUM;  // SW2  
    if (switches & (1 << SW_l1)) return EASY;    // SW1
    return EASY; // default
}