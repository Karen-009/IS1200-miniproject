# include "minesweeper.h"
# include <stdio.h>
# include <string.h>

void init_game(int difficulty);
void draw_board(void);
void handle_input(void);
void draw_game_over(void);
void delay(int cycles);

int main(void) {
    busy_wait(100000);

    Difficulty diff = choose_difficulty_from_switches();
    start_new_game(diff);
    render_board();
    uint32_t prev_keys = 0;

    while (1) {
        render_board();
        uint32_t sw = read_switches();
        uint32_t keys = read_keys();

        uint32_t key_pressed = keys & (1u << KEY_enter);
        uint32_t prev_key_pressed = prev_keys & (1u << KEY_enter);

        if (key_pressed && !prev_key_pressed) {
            if (sw & SW_MASK(SW_up)) {
                if (cursor_r > 0) cursor_r--;
            } else if (sw & SW_MASK(SW_down)) {
                if (cursor_r < g_rows - 1) cursor_r++;
            } else if (sw & SW_MASK(SW_left)) {
                if (cursor_c > 0) cursor_c--;
            } else if (sw & SW_MASK(SW_right)) {
                if (cursor_c < g_cols - 1) cursor_c++;
            } else if (sw & SW_MASK(SW_ACTION_1)) {
                toggle_flag(cursor_r, cursor_c);
            } else if (sw & SW_MASK(SW_ACTION_2)) {
                reveal_cell(cursor_r, cursor_c);
            }
        }

        if (game_over != 0) {
            render_board();
            while (!(read_keys() & (1u << KEY_enter))) { busy_wait(1000); }
            wait_key_release_all();
            diff = choose_difficulty_from_switches();
            start_new_game(diff);
            prev_keys = 0;
            continue;
        }

        prev_keys = keys;
        busy_wait(30000);
    }

    return 0;
}

void handle_interrupt(){

}