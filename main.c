# include "main_menu.h"
# include "sudoku.h"



int main() {
    init_main_menu();
    
    while(1) {
        switch(menu_state) {
            case MENU_STATE_MAIN:
                draw_main_menu(game_selection);
                menu_state = handle_menu_input();
                delay(100);
                break;
                
            case MENU_STATE_MINEWEEPER:
                run_minesweeper();
                break;
                
            case MENU_STATE_SUDOKU:
                run_sudoku();
                break;
        }
    }
    
    return 0;
}