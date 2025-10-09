#include "main_menu.h"
#include "sudoku.h"

int main() {
    init_main_menu();
    int last_selection = -1; // To track last selection for redraw optimization

    while(1) {
        switch(menu_state) {
            case MENU_STATE_MAIN:
                if (game_selection != last_selection) {
                    draw_main_menu(game_selection);
                    last_selection = game_selection;
                }
                menu_state = handle_menu_input();
                delay(1); // Small delay for responsive input, no flicker
                break;

            case MENU_STATE_MINEWEEPER:
                run_minesweeper();
                last_selection = -1; // Reset so menu redraws after returning
                break;

            case MENU_STATE_SUDOKU:
                run_sudoku();
                last_selection = -1; // Reset so menu redraws after returning
                break;
        }
    }

    return 0;
}