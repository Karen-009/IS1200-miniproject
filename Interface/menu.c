#include "main_menu.h"
#include "minesweeper.h"
#include "dtekv_board.h"  // Uncomment when you have Sudoku header
#include "sudoku_input_vga.h"
#include "sudoku_puzzles.h"
#include "sudoku_vga.h"
#include"sudoku.h"

volatile char *VGA = (volatile char *) VGA_Buffer;
volatile int *VGA_ctrl = (volatile int*) VGA_DMA;
volatile int *SWITCHES = (volatile int *) SWITCH_base;
volatile int *keys1 = (volatile int *) KEY1_base;

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
}

int handle_menu_input(void) {
    int current_switches = *SWITCHES;
    int current_keys = *keys1;
    static int last_switches = 0;
    static int last_keys = 0;
    
    // Handle game selection with switch 0
    if (current_switches & (1 << SW_SELECT_GAME)) {
        game_selection = MENU_SUDOKU;
    } else {
        game_selection = MENU_MINEWEEPER;
    }
    
    // Handle selection confirmation with KEY0
    if ((current_keys & (1 << KEY_enter)) && !(last_keys & (1 << KEY_enter))) {
        if (game_selection == MENU_MINEWEEPER) {
            return MENU_STATE_MINEWEEPER;
        } else {
            return MENU_STATE_SUDOKU;
        }
    }
    
    last_switches = current_switches;
    last_keys = current_keys;
    
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

void run_sudoku(void) {
    // Initialize and run sudoku game
    // This function will call your Sudoku game's main logic
    // For now, just display a placeholder
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

SudokuDifficulty get_selected_difficulty(void) {
    volatile int *SWITCHES = (volatile int *) SWITCH_base;
    int switches = *SWITCHES;

    if (switches & (1 << SW_l3)) return HARD;
    if (switches & (1 << SW_l2)) return MEDIUM;
    if (switches & (1 << SW_l1)) return EASY;
    return EASY; // Default to EASY if no switch is set
}