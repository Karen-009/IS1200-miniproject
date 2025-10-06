# include "minesweeper.h"
# include <stdio.h>
# include <time.h>
# include <string.h>

int main() {
    init_game(0);
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
                draw_game_over();
            }
            last_state = current_state;
        }
       
        // Simple delay
        delay(1);
    }
   
    return 0;
}