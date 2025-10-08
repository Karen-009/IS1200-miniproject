# include "minesweeper.h"
# include <stdio.h>
# include <time.h>
# include <string.h>

void init_game(int difficulty);
void draw_board(void);
void handle_input(void);
void draw_game_over(void);
void delay(int cycles);

int main() {
    init_game(0);
   
    while(1) {
        handle_input();
        draw_board();
        if (game.game_over || game.game_won)
        {
            draw_game_over();
        }
        delay(1);
    }
   
    return 0;
}