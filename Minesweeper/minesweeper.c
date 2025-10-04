<<<<<<< HEAD
# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <time.h>
# include <stdbool.h>
=======
# include <math.h>
# include <stdlib.h>
# include "system.h"
>>>>>>> 39ea9096b9f526b97150c5ff5c5d2098634f6e5a

int count_adjacent_mines(int x, int y);
void place_mine(int first_x, int first_y);
void flood_fill(int x, int y);


//VGA Memory Addresses
volatile char *VGA = (volatile char *) VGA_Buffer;
volatile int *VGA_ctrl = (volatile int*) VGA_DMA;

//Switch and Key Memory Addresses
volatile int *SWITCHES = (volatile int *) SWITCH_base;
volatile int *keys1 = (volatile int *) KEY1_base; //address of KEY1

//Game States
typedef struct {
    int grid[MAX_SIZE][MAX_SIZE];      // -1 = mine, 0-8 = adjacent mines
    int revealed[MAX_SIZE][MAX_SIZE];  // 1 = cell is shown, 0 = hidden
    int flagged[MAX_SIZE][MAX_SIZE];   // 1 = cell has flag, 0 = no flag
    int board_size;                    // Current grid size (8, 12, or 16)
    int mine_count;                    // Number of mines
    int game_over;                     // 1 = lost, 0 = still playing
    int game_won;                      // 1 = won, 0 = still playing
    int first_click;                   // Special handling for first move
    int cursor_x, cursor_y;            // Player's current position
    int last_switches, last_keys;      // Previous input states
} GameState;

GameState game;

// Compact digit representation using bits (5 bytes per digit)
unsigned char digits_compact[10][5] = {
    {0x1F, 0x11, 0x11, 0x11, 0x1F}, // 0
    {0x04, 0x06, 0x04, 0x04, 0x1F}, // 1
    {0x1F, 0x10, 0x1F, 0x01, 0x1F}, // 2
    {0x1F, 0x10, 0x1F, 0x10, 0x1F}, // 3
    {0x11, 0x11, 0x1F, 0x10, 0x10}, // 4
    {0x1F, 0x01, 0x1F, 0x10, 0x1F}, // 5
    {0x1F, 0x01, 0x1F, 0x11, 0x1F}, // 6
    {0x1F, 0x10, 0x08, 0x04, 0x02}, // 7
    {0x1F, 0x11, 0x1F, 0x11, 0x1F}, // 8
};

void init_game(int difficlty){
    game.first_click = 1;
    game.game_over = 0;
    game.game_won = 0;
    game.cursor_x = 0;
    game.cursor_y = 0;
    game.last_switches = 0;
    game.last_keys = 0;

    // Set the board size and mine count based on difficulty
    switch (difficlty){
        case 0: //Level 1
            game.board_size = EASY_SIZE;
            game.mine_count = EASY_MINES;
            break;
        case 1: //Level 2
            game.board_size = MEDIUM_SIZE;
            game.mine_count = MEDIUM_MINES;
            break;  
        case 2: //Level 3
            game.board_size = HARD_SIZE;
            game.mine_count = HARD_MINES;
            break;
        default:
            game.board_size = EASY_SIZE;
            game.mine_count = EASY_MINES;
    }

    //Clears all data and resets the game to initial state, runs though all of the cells and sets to initial state
    for(int i = 0; i < MAX_SIZE; i++){
        for(int j = 0; j < MAX_SIZE; j++){
            game.grid[i][j] = 0;
            game.revealed[i][j] = 0;
            game.flagged[i][j] = 0;
        }
    }
}

void setup_board(int first_x, int first_y){
    int placed_mines_count = 0;

    //Place mines
    while (placed_mines_count < game.mine_count)
    {
        //Generates random locations for the mines within the board size
        int x = rand() % game.board_size;
        int y = rand() % game.board_size;

        if((x == first_x && y == first_y) || game.grid[x][y] == -1){
            continue; // Skip if it's the first click or already a mine
        }

        game.grid[x][y] = -1; // A mine is placed at the location (x,y), in the 2D array
        placed_mines_count++;
    }

    for (int i = 0; i < game.board_size; i++){
        for(int j = 0; j < game.board_size; j++){
            if(game.grid[i][j] != -1){ // If the cell does not contain a mine, jump to counting adjusent mines
                game.grid[i][j] = count_adjacent_mines(i, j);
            }
        }
    }
}

// Count the number of mines adjacent to the position (x, y) by looking at x-1, x+1, y-1, x+1 etc. for each cell that != -1
int count_adjacent_mines(int x, int y){
    int count = 0;
    for(int dy = -1; dy <= 1; dy++){
        for(int dx = -1; dx <= 1; dx ++){
            int new_x = x + dx;
            int new_y = y + dy;

            if(new_x >=0 && new_x < game.board_size && new_y >=0 && new_y < game.board_size){
                if(game.grid[new_x][new_y] == -1){
                    count++;
                }
            }
        }
    }
    return count; //Returns the number of adjacent mines to be written into the cell of game.grind[i][j]
}

void draw_pixel(int x, int y, char color){
    if(x >=0 && x < 320 && y  >=0 && y < 240){
        VGA[y * 320 + x] = color;
    }
}

void draw_rect(int x, int y, int width, int height, char color){
    for (int i = 0; i < width; i++){
        for (int j = 0; j < height; j++){
            draw_pixel(x, y, color);
        }
    }
}

void draw_block(int x, int y, int width, int height, char color){
    for (int dx = 0; dx < width; dx ++){
        for (int dy = 0; dy < height; dy++){
            draw_pixel(x+dx, y+dy, color);
        }
    }
}

// Checks if a bit is set in the compact representation
int is_pixel_set(unsigned char row, int bit_pos){
    return (row >> (4 - bit_pos)) & 1;
}

void draw_number(int grid_x, int grid_y, int number){
    if(number < 0 || number > 8) return; // Only supports single digits 0-8

    int cell_size = 20;

    //Calculate picel coorinates with 1-pixel margin
    int pixel_x = grid_x * cell_size + 1;
    int pixel_y = grid_y * cell_size + 1;

    int draw_size = cell_size - 2;
    int scale = draw_size/5;

    if(scale < 1) scale = 1; // Ensure at least 1 pixel per bit

    //Center the number in the
    int offset_x = (draw_size - (5 * scale)) / 2;
    int offset_y = (draw_size - (7 * scale)) / 2;

    int start_x = pixel_x + offset_x;
    int start_y = pixel_y + offset_y;

    char color;

    switch(number) {
        case 1: color = blue; break;
        case 2: color = green; break;
        case 3: color = red; break;
        case 4: color = magenta; break;
        case 5: color = yellow; break;
        case 6: color = dark_gray; break;
        case 7: color = black; break;
        case 8: color = cyan; break;
        default: color = gray;
    }
    
    for (int row = 0; row < 5; row++){
        for (int col = 0; col < 5; col++){
            if (is_pixel_set(digits_compact[number][row], col)){
                draw_block(start_x + col * scale, start_y + 
                            row * scale, scale, scale, color);
            }
        }
    }
}

void draw_cell(int x, int y, int cell_x, int cell_y){
    int cell_size = 20;
    int pixel_x = cell_x * cell_size; 
    int pixel_y = cell_y * cell_size;
    if(game.revealed[cell_x][cell_y]){
        draw_rect(pixel_x, pixel_y, cell_size, cell_size, white);
        // If the cell contains a mine drawa circle in the center
        if (game.grid[cell_x][cell_y] == -1){
            for (int i = 0; i < cell_size; i++){
                for (int j = 0; j < cell_size; j++){
                    int dx = i - cell_size/2;
                    int dy = j - cell_size/2;
                    if (dx * dx + dy * dy <= (cell_size/3) * (cell_size/3)){
                        draw_pixel(pixel_x + i, pixel_y + j, black);
                    }
                }
            }
        } else if(game.grid[cell_x][cell_y] > 0){
            draw_number(cell_x, cell_y, game.grid[cell_x][cell_y]);
        }
    } else {
        draw_rect(pixel_x, pixel_y, cell_size, cell_size, gray);

        for (int i = 0; i < cell_size; i++){
            draw_pixel(pixel_x + i, pixel_y, dark_gray); // Top border
            draw_pixel(pixel_x, pixel_y + i, dark_gray); // Left border
            draw_pixel(pixel_x + i, pixel_y + cell_size - 1, dark_gray); // Bottom border
            draw_pixel(pixel_x + cell_size - 1, pixel_y + i, dark_gray); // Right border
        }
    
        if (game.flagged[cell_x][cell_y]){
            // Draw a simple flag (triangle on a pole)
            for (int i = 0; i < cell_size; i++){
                draw_pixel(pixel_x + cell_size/2, pixel_y + i, black); // Flag pole
            }
            for (int i = 0; i < cell_size/2; i++){
                for (int j = 0; j < cell_size/4; j++){
                    if (j <= i){ // Simple triangle shape
                        draw_pixel(pixel_x + cell_size/2 + i, pixel_y + j + cell_size/4, red);
                    }
                }
            }
        }
    }
}

// Draw cursor around current cell
void draw_cursor() {
    int cell_size = 20;
    int screen_x = game.cursor_x * cell_size;
    int screen_y = game.cursor_y * cell_size;
    
    // Draw yellow border around cursor cell
    for(int i = 0; i < cell_size; i++) {
        draw_pixel(screen_x + i, screen_y, yellow);
        draw_pixel(screen_x + i, screen_y + cell_size - 1, yellow);
        draw_pixel(screen_x, screen_y + i, yellow);
        draw_pixel(screen_x + cell_size - 1, screen_y + i, yellow);
    }
}

void draw_board(){
    draw_rect(0, 0, 320, 240, light_blue); // Clear screen

    // Draw each cell
    for(int i = 0; i < game.board_size; i++){
        for(int j = 0; game.board_size; j++){
            draw_cell(0, 0, i, j);
        }
    }
    draw_cursor(); // Draw the cursor on top
}

void reveal_cell(int x, int y){
    if(x <0 || x >= game.board_size || y < 0 || y >= game.board_size){
        return;
    }

    if(game.revealed[x][y] || game.flagged[x][y]){
        return; // Already revealed or flagged
    }

    //PLace mines after the first click so the first click is not a mine
    if(game.first_click){
        place_mine(x, y);
        game.first_click = 0;
    }

    game.revealed[x][y] = 1;

    if(game.grid[x][y] == -1){
        game.game_over = 1; //A mine has been hit
        return;
    }

    //If the cell doen't have any adjacent mines it reveals all of the adjacent cells
    if(game.grid[x][y] == 0){
        flood_fill(x, y);
    }
}

void flood_fill(int x, int y){
    for(int i = -1; i <= 1; i++){
        for(int j = -1; j <= 1; j++){
            int new_x = x + i;
            int new_y = y + j;

            if(new_x >=0 && new_x < game.board_size && new_y >=0 && new_y < game.board_size && !game.revealed[new_x][new_y]){
                game.revealed[new_x][new_y] = 1;
                if(game.grid[new_x][new_y] == 0){
                    flood_fill(new_x, new_y); // Recursively reveal adjacent cells
                }
            }
        }
    }
}

void flag(int x, int y){
    if(!game.revealed[x][y]){
        game.revealed[x][y] = !game.flagged[x][y];
    }
}

void check_win_condition(){
    int correct_flaggs = 0;
    int total_cells = game.board_size * game.board_size;
    int revealed_cells = 0;

    //Going though all of the cells to count the number of revealed cells and correct flaggs
    for(int i = 0; i < game.board_size; i++){
        for(int j = 0; j < game.board_size; j++){
            if(game.revealed[i][j]){
                revealed_cells++;
            }

            if (game.flagged[i][j] == -1 && game.grid[i][j]){
                correct_flaggs++;
            }
        }
    }

    if(correct_flaggs == game.mine_count || revealed_cells == total_cells - game.mine_count){
        game.game_won = 1;
    }
}

int get_level(){
    int switches = *SWITCHES;

    if(switches & (1 << SW_l1)){
        return 0;
    }
    if(switches & (1 << SW_l2)){
        return 1;
    }
    if(switches & (1 << SW_l3)){
        return 2;
    }

    return 0; // Default to level 1
}

void move_cursor(int dx, int dy){
    int new_x = game.cursor_x + dx;
    int new_y = game.cursor_y + dy;

    if(new_x >= 0 && new_x < game.board_size){
        game.cursor_x = new_x;
    }
    if(new_y >= 0 && new_y < game.board_size){
        game.cursor_y = new_y;
    }
}

void process_action(int action){
    if(game.game_over || game.game_won){
        return; // No actions allowed if game is over or won
    }

    switch (action)
    {
    case SW_flag: 
        flag(game.cursor_x, game.cursor_y);
        break;
    case SW_reveal:
        reveal_cell(game.cursor_x, game.cursor_y);
        break;
    }
    check_win_condition();
}

void handel_input(){
    int current_switches = *SWITCHES;
    int current_keys = *keys1;

    static int last_difficulty = -1;
    int current_difficulty = get_level();
    if(current_difficulty != last_difficulty){
        init_game(current_difficulty);
        last_difficulty = current_difficulty;
        return;
    }

    //Check for movement inputs, if the switch is on and key1 is pressed
    if((current_keys & (1 << KEY_enter)) && !(game.last_keys & (1 << KEY_enter))){
        if((current_switches & (1 << SW_up)) && !(game.last_switches & (1 << SW_up))){
            move_cursor(0, -1);
        }
        if((current_switches & (1 << SW_down)) && !(game.last_switches & (1 << SW_down))){
            move_cursor(0, 1);
        }
        if((current_switches & (1 << SW_left)) && !(game.last_switches & (1 << SW_left))){
            move_cursor(-1, 0);
        }
        if((current_switches & (1 << SW_right)) && !(game.last_switches & (1 << SW_right))){
            move_cursor(1, 0);
        }
        if((current_switches & (1 << SW_flag)) && !(game.last_switches & (1 << SW_flag))){
            process_action(SW_flag);
        }
        if((current_switches & (1 << SW_reveal)) && !(game.last_switches & (1 << SW_reveal))){
            process_action(SW_reveal);
        }
    }
    game.last_switches = current_switches;
    game.last_keys = current_keys;
}

void draw_game_over(){
    if (game.game_over){
        draw_rect(80, 200, 160, 80, red);
    } else if (game.game_won){
        draw_rect(80, 200, 160, 80, green);
    }
}

void delay(int cycles){
    for(int i = 0; i < cycles * 10000; i++){
        __asm volatile ("nop"); // No operation, just waste time
    }
}