// Zunjee, Karen main program file
#include "main_menu.h"
#include "sudoku.h"

int main() {
    init_main_menu();
    int last_selection = -1; // To track last selection for redraw optimization, -1 means none

    while(1) {  
        switch(menu_state) {    
            case MENU_STATE_MAIN:   
                if (game_selection != last_selection) {     // Only redraw if selection changed
                    draw_main_menu(game_selection);         // Draw menu with current selection
                    last_selection = game_selection;        // Update last selection
                }
                menu_state = handle_menu_input();   // Check for menu input
                delay(1);     
                break;

            case MENU_STATE_MINEWEEPER:     // Run minesweeper game
                run_minesweeper();      // After game ends, return to menu
                last_selection = -1; // Reset so menu redraws after returning
                break;

            case MENU_STATE_SUDOKU:     // Run sudoku game
                run_sudoku();       // After game ends, return to menu
                last_selection = -1; // Reset so menu redraws after returning
                break;
        }
    }

    return 0;
}

void handle_interrupt(void) {   
    // Basic interrupt handler
    asm volatile("mret");
}