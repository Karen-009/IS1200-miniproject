# include "minesweeper.c"
# include "system.h"
# include <stdio.h>
# include <time.h>
# include <string.h>

int main(){
    unsigned int y_ofs = 0;

    init_game(0); // Start with easy level
    draw_board();

    while (1)
    {
        // Update the VGA display
        *(VGA_ctrl +1 ) = (unsigned int) (VGA_Buffer + y_ofs * 320);
        *(VGA_ctrl + 0) = 0;
        y_ofs = (y_ofs + 1) % 240; // Vertical scrolling effect

        handel_input();

        // Redraw board if game state changed
        static int last_cursor_x = -1, last_cursor_y = -1;
        static int last_game_state = 0;

        if (game.cursor_x != last_cursor_x || game.cursor_y != last_cursor_y || game.game_over != last_game_state || game.game_won != (last_game_state >> 1)){
            draw_board();

            if(game.game_over || game.game_won){
                draw_game_over();
            }
            last_cursor_x = game.cursor_x;
            last_cursor_y = game.cursor_y;
            last_game_state = game.game_over | (game.game_won << 1);
        }
        delay(100000);
    }
    return 0;
}