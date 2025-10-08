#include "new.h"

// Simple random number generator (Xorshift)
unsigned int custom_rand(MinesweeperGame* game) {
    game->random_seed ^= game->random_seed << 13;
    game->random_seed ^= game->random_seed >> 17;
    game->random_seed ^= game->random_seed << 5;
    return game->random_seed;
}

void custom_srand(MinesweeperGame* game, unsigned int seed) {
    game->random_seed = seed;
}

// Initialize game
void init_game(MinesweeperGame* game, int difficulty) {
    game->difficulty = difficulty;
    game->game_state = STATE_PLAYING;
    game->cursor_x = 0;
    game->cursor_y = 0;
    game->flags_placed = 0;
    game->cells_revealed = 0;
    game->move_count = 0;
    game->move_pending = 0;
    game->last_move_x = 0;
    game->last_move_y = 0;
    
    // Use timer as random seed
    volatile int* timer = (volatile int*)TIMER_base;
    custom_srand(game, *timer);
    
    // Set mine count based on difficulty
    switch(difficulty) {
        case DIFFICULTY_EASY:
            game->mine_count = 10;
            break;
        case DIFFICULTY_MEDIUM:
            game->mine_count = 40;
            break;
        case DIFFICULTY_HARD:
            game->mine_count = 99;
            break;
    }
    
    // Initialize grid
    for(int y = 0; y < GRID_SIZE; y++) {
        for(int x = 0; x < GRID_SIZE; x++) {
            game->grid[y][x] = CELL_HIDDEN;
        }
    }
    
    place_mines(game);
}

// Place mines randomly
void place_mines(MinesweeperGame* game) {
    int mines_placed = 0;
    while(mines_placed < game->mine_count) {
        int x = custom_rand(game) % GRID_SIZE;
        int y = custom_rand(game) % GRID_SIZE;
        
        // Don't place mine on first click position
        if((x != game->cursor_x || y != game->cursor_y) && 
           !(game->grid[y][x] & CELL_MINE)) {
            game->grid[y][x] |= CELL_MINE;
            mines_placed++;
        }
    }
}

// Count adjacent mines
int count_adjacent_mines(MinesweeperGame* game, int x, int y) {
    int count = 0;
    for(int dy = -1; dy <= 1; dy++) {
        for(int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if(nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
                if(game->grid[ny][nx] & CELL_MINE) {
                    count++;
                }
            }
        }
    }
    return count;
}

// Reveal cell (recursive for empty cells)
void reveal_cell(MinesweeperGame* game, int x, int y) {
    if(x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return;
    if(game->grid[y][x] & CELL_REVEALED) return;
    if(game->grid[y][x] & CELL_FLAGGED) return;
    
    game->grid[y][x] |= CELL_REVEALED;
    game->cells_revealed++;
    
    if(game->grid[y][x] & CELL_MINE) {
        game->grid[y][x] |= CELL_EXPLODED;
        game->game_state = STATE_LOST;
        return;
    }
    
    // If empty cell, reveal neighbors
    int adjacent = count_adjacent_mines(game, x, y);
    if(adjacent == 0) {
        for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -1; dx <= 1; dx++) {
                if(dx != 0 || dy != 0) {
                    reveal_cell(game, x + dx, y + dy);
                }
            }
        }
    }
}

// Toggle flag on cell
void toggle_flag(MinesweeperGame* game, int x, int y) {
    if(game->grid[y][x] & CELL_REVEALED) return;
    
    if(game->grid[y][x] & CELL_FLAGGED) {
        game->grid[y][x] &= ~CELL_FLAGGED;
        game->flags_placed--;
    } else {
        game->grid[y][x] |= CELL_FLAGGED;
        game->flags_placed++;
    }
}

// Draw the entire game
void draw_game(MinesweeperGame* game) {
    // Clear screen
    for(int i = 0; i < VGA_BUFFER_SIZE; i++) {
        *((volatile unsigned char*)(VGA_Buffer + i)) = black;
    }
    
    // Draw grid background
    for(int y = 0; y < GRID_SIZE; y++) {
        for(int x = 0; x < GRID_SIZE; x++) {
            int screen_x = GRID_OFFSET_X + x * CELL_SIZE;
            int screen_y = GRID_OFFSET_Y + y * CELL_SIZE;
            
            // Draw cell border
            for(int py = 0; py < CELL_SIZE; py++) {
                for(int px = 0; px < CELL_SIZE; px++) {
                    int addr = (screen_y + py) * VGA_WIDTH + (screen_x + px);
                    if(px == 0 || px == CELL_SIZE-1 || py == 0 || py == CELL_SIZE-1) {
                        *((volatile unsigned char*)(VGA_Buffer + addr)) = dark_gray;
                    }
                }
            }
        }
    }
    
    // Draw cells
    for(int y = 0; y < GRID_SIZE; y++) {
        for(int x = 0; x < GRID_SIZE; x++) {
            draw_cell(game, x, y);
        }
    }
    
    // Draw cursor
    draw_cursor(game);
    
    // Draw UI
    draw_ui(game);
}

// Draw individual cell
void draw_cell(MinesweeperGame* game, int x, int y) {
    int screen_x = GRID_OFFSET_X + x * CELL_SIZE + 1;
    int screen_y = GRID_OFFSET_Y + y * CELL_SIZE + 1;
    int cell_color;
    
    int cell_state = game->grid[y][x];
    
    if(cell_state & CELL_REVEALED) {
        if(cell_state & CELL_MINE) {
            cell_color = (cell_state & CELL_EXPLODED) ? red : magenta;
        } else {
            cell_color = light_gray;
        }
    } else if(cell_state & CELL_FLAGGED) {
        cell_color = yellow;
    } else {
        cell_color = blue;
    }
    
    // Fill cell background
    for(int py = 1; py < CELL_SIZE-1; py++) {
        for(int px = 1; px < CELL_SIZE-1; px++) {
            int addr = (screen_y + py) * VGA_WIDTH + (screen_x + px);
            *((volatile unsigned char*)(VGA_Buffer + addr)) = cell_color;
        }
    }
    
    // Draw number if revealed and has adjacent mines
    if((cell_state & CELL_REVEALED) && !(cell_state & CELL_MINE)) {
        int adjacent = count_adjacent_mines(game, x, y);
        if(adjacent > 0) {
            // Simple number drawing using pixels
            int center_x = screen_x + (CELL_SIZE-2)/2;
            int center_y = screen_y + (CELL_SIZE-2)/2;
            
            // Choose color based on number
            int number_color;
            switch(adjacent) {
                case 1: number_color = blue; break;
                case 2: number_color = green; break;
                case 3: number_color = red; break;
                case 4: number_color = dark_blue; break;
                case 5: number_color = brown; break;
                case 6: number_color = cyan; break;
                case 7: number_color = black; break;
                case 8: number_color = dark_gray; break;
                default: number_color = black; break;
            }
            
            // Draw a simple dot pattern for numbers
            for(int i = -1; i <= 1; i++) {
                for(int j = -1; j <= 1; j++) {
                    if(i == 0 || j == 0) { // Simple cross pattern
                        int addr = (center_y + j) * VGA_WIDTH + (center_x + i);
                        *((volatile unsigned char*)(VGA_Buffer + addr)) = number_color;
                    }
                }
            }
        }
    }
}

// Draw cursor
void draw_cursor(MinesweeperGame* game) {
    int screen_x = GRID_OFFSET_X + game->cursor_x * CELL_SIZE;
    int screen_y = GRID_OFFSET_Y + game->cursor_y * CELL_SIZE;
    
    // Draw cursor border (thicker for visibility)
    for(int py = 0; py < CELL_SIZE; py++) {
        for(int px = 0; px < CELL_SIZE; px++) {
            if(px < 3 || px > CELL_SIZE-4 || py < 3 || py > CELL_SIZE-4) {
                int addr = (screen_y + py) * VGA_WIDTH + (screen_x + px);
                *((volatile unsigned char*)(VGA_Buffer + addr)) = white;
            }
        }
    }
}

// Draw UI information
void draw_ui(MinesweeperGame* game) {
    // Draw mines remaining in top left
    int mines_remaining = game->mine_count - game->flags_placed;
    
    // Simple number display for mines remaining
    int display_x = 10;
    int display_y = 5;
    
    // Convert number to digits and display
    int temp = mines_remaining;
    int digits[3] = {0};
    
    if(temp < 0) temp = 0;
    if(temp > 999) temp = 999;
    
    digits[0] = temp / 100;
    digits[1] = (temp % 100) / 10;
    digits[2] = temp % 10;
    
    // Draw background for counter
    for(int y = 0; y < 10; y++) {
        for(int x = 0; x < 30; x++) {
            int addr = (display_y + y) * VGA_WIDTH + (display_x + x);
            *((volatile unsigned char*)(VGA_Buffer + addr)) = dark_gray;
        }
    }
    
    // Draw game status text
    int status_x = VGA_WIDTH - 100;
    int status_y = 5;
    
    // Draw status background
    for(int y = 0; y < 10; y++) {
        for(int x = 0; x < 80; x++) {
            int addr = (status_y + y) * VGA_WIDTH + (status_x + x);
            if(game->game_state == STATE_WON) {
                *((volatile unsigned char*)(VGA_Buffer + addr)) = green;
            } else if(game->game_state == STATE_LOST) {
                *((volatile unsigned char*)(VGA_Buffer + addr)) = red;
            } else {
                *((volatile unsigned char*)(VGA_Buffer + addr)) = blue;
            }
        }
    }
}

// Handle input from switches and keys
void handle_input(MinesweeperGame* game) {
    volatile int* switches = (volatile int*)SWITCH_BASE;
    volatile int* keys = (volatile int*)KEY1_base;
    int switch_state = *switches;
    int key_state = *keys;
    
    static int last_key_state = 0;
    static int move_queued = 0;
    
    // Check for difficulty selection (resets game)
    if(switch_state & (1 << SW_l1)) {
        if(game->difficulty != DIFFICULTY_EASY || game->game_state != STATE_PLAYING) {
            reset_game(game);
            game->difficulty = DIFFICULTY_EASY;
        }
    } else if(switch_state & (1 << SW_l2)) {
        if(game->difficulty != DIFFICULTY_MEDIUM || game->game_state != STATE_PLAYING) {
            reset_game(game);
            game->difficulty = DIFFICULTY_MEDIUM;
        }
    } else if(switch_state & (1 << SW_l3)) {
        if(game->difficulty != DIFFICULTY_HARD || game->game_state != STATE_PLAYING) {
            reset_game(game);
            game->difficulty = DIFFICULTY_HARD;
        }
    }
    
    // Handle movement with double-tap
    if(key_state & (1 << KEY_enter)) {
        if(!(last_key_state & (1 << KEY_enter))) {
            // First press - queue move
            game->move_pending = 1;
            game->last_move_x = game->cursor_x;
            game->last_move_y = game->cursor_y;
            move_queued = 1;
        } else if(move_queued) {
            // Second press - execute move twice
            game->move_pending = 2;
            move_queued = 0;
        }
    } else {
        move_queued = 0;
    }
    
    last_key_state = key_state;
    
    // Execute moves
    if(game->move_pending > 0 && game->game_state == STATE_PLAYING) {
        int move_x = 0, move_y = 0;
        
        if(switch_state & (1 << SW_up)) move_y = -1;
        if(switch_state & (1 << SW_down)) move_y = 1;
        if(switch_state & (1 << SW_left)) move_x = -1;
        if(switch_state & (1 << SW_right)) move_x = 1;
        
        for(int i = 0; i < game->move_pending; i++) {
            int new_x = game->cursor_x + move_x;
            int new_y = game->cursor_y + move_y;
            
            if(new_x >= 0 && new_x < GRID_SIZE) game->cursor_x = new_x;
            if(new_y >= 0 && new_y < GRID_SIZE) game->cursor_y = new_y;
        }
        
        game->move_pending = 0;
    }
    
    // Handle actions
    static int last_action1 = 0, last_action2 = 0;
    
    if((switch_state & (1 << SW_ACTION_1)) && game->game_state == STATE_PLAYING) {
        if(!last_action1) {
            toggle_flag(game, game->cursor_x, game->cursor_y);
        }
        last_action1 = 1;
    } else {
        last_action1 = 0;
    }
    
    if((switch_state & (1 << SW_ACTION_2)) && game->game_state == STATE_PLAYING) {
        if(!last_action2) {
            reveal_cell(game, game->cursor_x, game->cursor_y);
            if(check_win_condition(game)) {
                game->game_state = STATE_WON;
            }
        }
        last_action2 = 1;
    } else {
        last_action2 = 0;
    }
}

// Check win condition
int check_win_condition(MinesweeperGame* game) {
    return (game->cells_revealed == (GRID_SIZE * GRID_SIZE - game->mine_count));
}

// Reset game
void reset_game(MinesweeperGame* game) {
    init_game(game, game->difficulty);
}