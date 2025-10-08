#include "test.h"
#include "dtekv_board.h"

//VGA Memory Addresses
volatile char *VGA = (volatile char *) VGA_Buffer;
volatile int *VGA_ctrl = (volatile int*) VGA_DMA;

//Switch and Key Memory Addresses
volatile int *SWITCHES = (volatile int *) SWITCH_base;
volatile int *keys1 = (volatile int *) KEY1_base;

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
    {0x1F, 0x11, 0x1F, 0x10, 0x1F}  // 9
};

GameState game;

// Custom utility functions (since we can't use stdlib)
void memory_set(void* ptr, char value, int size) {
    char* p = (char*)ptr;
    for (int i = 0; i < size; i++) {
        p[i] = value;
    }
}

unsigned int custom_rand(unsigned int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int custom_strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void init_game(int difficulty){
    // Initialize the entire game structure to zero
    memory_set(&game, 0, sizeof(GameState));
    
    game.first_click = 1;
    game.game_over = 0;
    game.game_won = 0;
    game.cursor_x = 0;
    game.cursor_y = 0;
    game.last_switches = 0;
    game.last_keys = 0;
    
    // Set the board size and mine count based on difficulty
    switch (difficulty){
        case 0: // Easy
            game.board_size = EASY_SIZE;
            game.mine_count = EASY_MINES;
            break;
        case 1: // Medium  
            game.board_size = MEDIUM_SIZE;
            game.mine_count = MEDIUM_MINES;
            break;  
        case 2: // Hard
            game.board_size = HARD_SIZE;
            game.mine_count = HARD_MINES;
            break;
        default:
            game.board_size = EASY_SIZE;
            game.mine_count = EASY_MINES;
    }

    // Clear all data and reset the game to initial state
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
    unsigned int seed = *((volatile unsigned int*)TIMER_base); // Use timer as random seed

    // Place mines
    while (placed_mines_count < game.mine_count)
    {
        // Generate random locations for the mines within the board size
        int x = custom_rand(&seed) % game.board_size;
        int y = custom_rand(&seed) % game.board_size;

        if((x == first_x && y == first_y) || game.grid[x][y] == -1){
            continue; // Skip if it's the first click or already a mine
        }

        game.grid[x][y] = -1; // A mine is placed at the location (x,y)
        placed_mines_count++;
    }

    // Calculate adjacent mines for all cells
    for (int i = 0; i < game.board_size; i++){
        for(int j = 0; j < game.board_size; j++){
            if(game.grid[i][j] != -1){ 
                game.grid[i][j] = count_adjacent_mines(i, j);
            }
        }
    }
}

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
    return count;
}

void draw_pixel(int x, int y, char color){
    if(x >=0 && x < 640 && y >=0 && y < 480){
        VGA[y * 640 + x] = color;
    }
}

void draw_rect(int x, int y, int width, int height, char color){
    for (int i = 0; i < width; i++){
        for (int j = 0; j < height; j++){
            draw_pixel(x + i, y + j, color);
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

void draw_digit(int grid_x, int grid_y, int number, char color) {
    if(number < 0 || number > 8) return;

    int cell_size = 20;
    int pixel_x = grid_x * cell_size;
    int pixel_y = grid_y * cell_size;

    // Center the 5x5 digit in the cell with scale 3
    int margin = (cell_size - 5*3) / 2;
    int start_x = pixel_x + margin;
    int start_y = pixel_y + margin;

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            if ((digits_compact[number][row] >> (4 - col)) & 1) {
                // 3x3 block per "pixel"
                for(int dx = 0; dx < 3; dx++)
                    for(int dy = 0; dy < 3; dy++)
                        draw_pixel(start_x + col*3 + dx, start_y + row*3 + dy, color);
            }
        }
    }
}

void draw_cell(int cell_x, int cell_y){
    int cell_size = 20;
    int pixel_x = cell_x * cell_size;
    int pixel_y = cell_y * cell_size;

    if(game.revealed[cell_x][cell_y]){
        draw_rect(pixel_x, pixel_y, cell_size, cell_size, white);
        
        if (game.grid[cell_x][cell_y] == -1){
            // Draw mine as black circle
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
            // Draw number with appropriate color
            char color;
            switch(game.grid[cell_x][cell_y]) {
                case 1: color = blue; break;
                case 2: color = green; break;
                case 3: color = red; break;
                case 4: color = dark_blue; break;
                case 5: color = brown; break;
                case 6: color = cyan; break;
                case 7: color = black; break;
                case 8: color = dark_gray; break;
                default: color = gray;
            }
            draw_digit(cell_x, cell_y, game.grid[cell_x][cell_y], color);
        }
    } else {
        // Hidden cell
        draw_rect(pixel_x, pixel_y, cell_size, cell_size, light_gray);

        // Draw 3D effect borders
        for (int i = 0; i < cell_size; i++){
            draw_pixel(pixel_x + i, pixel_y, white); // Top border (light)
            draw_pixel(pixel_x, pixel_y + i, white); // Left border (light)
            draw_pixel(pixel_x + i, pixel_y + cell_size - 1, dark_gray); // Bottom border (dark)
            draw_pixel(pixel_x + cell_size - 1, pixel_y + i, dark_gray); // Right border (dark)
        }
   
        if (game.flagged[cell_x][cell_y]){
            // Draw flag
            for (int i = 0; i < cell_size; i++){
                draw_pixel(pixel_x + cell_size/2, pixel_y + i, black); // Flag pole
            }
            for (int i = 0; i < cell_size/2; i++){
                for (int j = 0; j < cell_size/4; j++){
                    if (j <= i){
                        draw_pixel(pixel_x + cell_size/2 + i, pixel_y + j + cell_size/4, red);
                    }
                }
            }
        }
    }
}

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
    // Clear screen with light blue background
    draw_rect(0, 0, 640, 480, light_blue);

    // Calculate board position to center it
    int board_pixel_size = game.board_size * 20;
    int start_x = (640 - board_pixel_size) / 2;
    int start_y = (480 - board_pixel_size) / 2;
    
    // Draw board background
    draw_rect(start_x - 2, start_y - 2, board_pixel_size + 4, board_pixel_size + 4, dark_gray);

    // Draw each cell
    for(int i = 0; i < game.board_size; i++){
        for(int j = 0; j < game.board_size; j++){
            int screen_x = start_x + i * 20;
            int screen_y = start_y + j * 20;
            
            // Temporary translation for drawing functions
            int temp_x = game.cursor_x;
            int temp_y = game.cursor_y;
            game.cursor_x = i;
            game.cursor_y = j;
            
            draw_cell(i, j);
            
            game.cursor_x = temp_x;
            game.cursor_y = temp_y;
        }
    }
    
    // Draw cursor on top
    int cursor_screen_x = start_x + game.cursor_x * 20;
    int cursor_screen_y = start_y + game.cursor_y * 20;
    for(int i = 0; i < 20; i++) {
        draw_pixel(cursor_screen_x + i, cursor_screen_y, yellow);
        draw_pixel(cursor_screen_x + i, cursor_screen_y + 19, yellow);
        draw_pixel(cursor_screen_x, cursor_screen_y + i, yellow);
        draw_pixel(cursor_screen_x + 19, cursor_screen_y + i, yellow);
    }
}

void reveal_cell(int x, int y){
    if(x < 0 || x >= game.board_size || y < 0 || y >= game.board_size){
        return;
    }

    if(game.revealed[x][y] || game.flagged[x][y]){
        return; // Already revealed or flagged
    }

    // Place mines after the first click so the first click is not a mine
    if(game.first_click){
        setup_board(x, y);
        game.first_click = 0;
    }

    game.revealed[x][y] = 1;

    if(game.grid[x][y] == -1){
        game.game_over = 1; // Mine hit
        return;
    }

    // If the cell doesn't have any adjacent mines, reveal all adjacent cells
    if(game.grid[x][y] == 0){
        flood_fill(x, y);
    }
}

void flood_fill(int x, int y){
    // Simple recursive flood fill (be careful with stack size)
    for(int dx = -1; dx <= 1; dx++){
        for(int dy = -1; dy <= 1; dy++){
            int nx = x + dx;
            int ny = y + dy;
            if(nx >= 0 && nx < game.board_size && ny >= 0 && ny < game.board_size){
                if(!game.revealed[nx][ny] && !game.flagged[nx][ny]){
                    game.revealed[nx][ny] = 1;
                    if(game.grid[nx][ny] == 0){
                        flood_fill(nx, ny);
                    }
                }
            }
        }
    }
}

void flag(int x, int y){
    if(!game.revealed[x][y]){
        game.flagged[x][y] = !game.flagged[x][y];
    }
}

void check_win_condition(){
    int revealed_cells = 0;
    for(int i = 0; i < game.board_size; i++){
        for(int j = 0; j < game.board_size; j++){
            if(game.revealed[i][j]) revealed_cells++;
        }
    }
    if(revealed_cells == game.board_size * game.board_size - game.mine_count){
        game.game_won = 1;
    }
}

int get_level(){
    int switches = *SWITCHES;

    if(switches & (1 << SW_l1)){
        return 0; // Easy
    }
    if(switches & (1 << SW_l2)){
        return 1; // Medium
    }
    if(switches & (1 << SW_l3)){
        return 2; // Hard
    }

    return 0; // Default to easy
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
        return;
    }

    switch (action)
    {
    case SW_ACTION_1:
        flag(game.cursor_x, game.cursor_y);
        break;
    case SW_ACTION_2:
        reveal_cell(game.cursor_x, game.cursor_y);
        break;
    }
    check_win_condition();
}

void handle_input(){
    int current_switches = *SWITCHES;
    int current_keys = *keys1;
    static int last_difficulty = -1;
    static int last_key_state = 0;
    static int last_move_time = 0;
    static int last_direction = 0;
    
    // Check for difficulty change
    int current_difficulty = get_level();
    if(current_difficulty != last_difficulty){
        init_game(current_difficulty);
        last_difficulty = current_difficulty;
        return;
    }
    
    // Check for key press (edge detection)
    int key_pressed = (current_keys & (1 << KEY_enter)) && !(last_key_state & (1 << KEY_enter));
    last_key_state = current_keys;
    
    if (key_pressed) {
        // Check if movement switches are active
        int move_switches_active = (current_switches & ((1 << SW_up) | (1 << SW_down) | (1 << SW_left) | (1 << SW_right))) != 0;
        
        if (move_switches_active) {
            // Handle movement with double-move feature
            int move_count = 1;
            int current_time = *((volatile int*)TIMER_base);
            int current_direction = current_switches & ((1 << SW_up) | (1 << SW_down) | (1 << SW_left) | (1 << SW_right));
            
            // Check for double movement (switch continuously on)
            if (current_direction == last_direction && (current_time - last_move_time) < 1000000) {
                move_count = 2; // Double move if same direction pressed quickly
            }
            
            last_move_time = current_time;
            last_direction = current_direction;
            
            for (int i = 0; i < move_count; i++) {
                if (current_switches & (1 << SW_up)) move_cursor(0, -1);
                if (current_switches & (1 << SW_down)) move_cursor(0, 1);
                if (current_switches & (1 << SW_right)) move_cursor(1, 0);
                if (current_switches & (1 << SW_left)) move_cursor(-1, 0);
            }
        } else {
            // Handle actions (flag or reveal)
            if (current_switches & (1 << SW_ACTION_1)) process_action(SW_ACTION_1);
            if (current_switches & (1 << SW_ACTION_2)) process_action(SW_ACTION_2);
        }
    }
}

void draw_game_over(){
    if (game.game_over){
        // Draw "GAME OVER" message
        draw_rect(200, 200, 240, 80, red);
    } else if (game.game_won){
        // Draw "YOU WIN" message  
        draw_rect(200, 200, 240, 80, green);
    }
}

void delay(int cycles){
    for(int i = 0; i < cycles * 10000; i++){
        __asm("nop");

    }
}

int main() {
    init_game(0); // Start with easy difficulty
    
    while(1) {
        handle_input();
        draw_board();
        
        if(game.game_over || game.game_won) {
            draw_game_over();
        }
        
        delay(1);
    }
    
    return 0;
}