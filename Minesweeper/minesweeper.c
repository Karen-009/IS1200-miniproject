# include "minesweeper.h"
# include "dtekv_board.h"

//VGA Memory Addresses
volatile char *VGA = (volatile char *) VGA_Buffer;
volatile int *VGA_ctrl = (volatile int*) VGA_DMA;
GameState game;

// Safe way to handle VGA control
void init_vga() {
    // Only write to control register, never read
    volatile int *VGA_ctrl = (volatile int*) VGA_DMA;
    *VGA_ctrl = 1;  // Start DMA
    
    // Don't store VGA_ctrl pointer globally if it causes issues
}

//Switch and Key Memory Addresses
volatile int *SWITCHES = (volatile int *) SWITCH_base;
volatile int *keys1 = (volatile int *) KEY1_base; //address of KEY1

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

void handle_interrupt(){}

void init_game(int dificulty){
    game.first_click = 1;
    game.game_over = 0;
    game.game_won = 0;
    game.cursor_x = 0;
    game.cursor_y = 0;
    game.last_switches = 0;
    game.last_keys = 0;

    // Set the board size and mine count based on difficulty
    switch (dificulty){
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


void draw_pixel(int x, int y, char color){
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)
    {
        VGA[y * VGA_WIDTH + x] = color;
    }
}

void clear_screen(char color){
    for (int i = 0; i < VGA_BUFFER_SIZE; i++)
    {
        VGA[i] = color;
    }
}

void draw_square (int x0, int y0, int width, int height, char color){
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            draw_pixel(x0 + x, y0 + y, color);
        }
    }
}

void draw_current_board() {
    for (int i = 0; i < game.board_size; i++) {
        for (int j = 0; j < game.board_size; j++) {
            draw_cell(j, i);
        }
    }
}

void draw_digit(int x, int y, int digit, char color) {
    if (digit < 0 || digit > 9) return;
    
    for (int row = 0; row < 5; row++) {
        unsigned char row_data = digits_compact[digit][row];
        for (int col = 0; col < 5; col++) {
            if (row_data & (1 << (4 - col))) {
                draw_pixel(x + col, y + row, color);
            }
        }
    }
}

void draw_cell(int grid_x, int grid_y) {
    int screen_x = grid_x * CELL_SIZE;
    int screen_y = grid_y * CELL_SIZE;
    
    // Choose color based on cell state
    char color;
    if (game.revealed[grid_y][grid_x]) {
        // Revealed cell
        if (game.grid[grid_y][grid_x] == -1) {
            color = 0x04; // Red for mine
        } else {
            color = 0x0F; // White for empty/revealed
        }
    } else {
        // Hidden cell
        if (game.flagged[grid_y][grid_x]) {
            color = 0x0E; // Yellow for flag
        } else {
            color = 0x08; // Gray for hidden
        }
    }
    
    // Draw the cell background
    draw_square(screen_x, screen_y, CELL_SIZE, CELL_SIZE, color);
    
    // If revealed and has adjacent mines, draw the number
    if (game.revealed[grid_y][grid_x] && game.grid[grid_y][grid_x] > 0) {
        draw_digit(screen_x + CELL_SIZE/2 - 2, screen_y + CELL_SIZE/2 - 2, 
                   game.grid[grid_y][grid_x], 0x00); // Black numbers
    }
    
    // Draw cursor (highlight current position)
    if (grid_x == game.cursor_x && grid_y == game.cursor_y) {
        // Draw a border around the cursor cell
        for (int i = 0; i < CELL_SIZE; i++) {
            draw_pixel(screen_x + i, screen_y, 0x0A); // Top border (green)
            draw_pixel(screen_x + i, screen_y + CELL_SIZE - 1, 0x0A); // Bottom border
            draw_pixel(screen_x, screen_y + i, 0x0A); // Left border
            draw_pixel(screen_x + CELL_SIZE - 1, screen_y + i, 0x0A); // Right border
        }
    }
}

void place_mines(int first_x, int first_y){
    int mines_placed = 0;

    while (mines_placed < game.mine_count)
    {
        int x = rand() % game.board_size;
        int y = rand() % game.board_size;

        if ((x != first_x || y != first_y) && game.grid[y][x] != -1) {
            game.grid[y][x] = -1;  // -1 represents a mine
            mines_placed++;
        }
    }
}

static int seed = 123456789;

void srand_custom(int s) {
    seed = s;
}

int rand() {
    seed = (1103515245 * seed + 12345) & 0x7fffffff;
    return (int)seed;
}

//Calculates and stores the number of adjacent mines in the game.grid[x][y]
void calculate_adjacent_mines() {
    int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    
    for (int y = 0; y < game.board_size; y++) {
        for (int x = 0; x < game.board_size; x++) {
            if (game.grid[y][x] != -1) {
                int count = 0;
                for (int i = 0; i < 8; i++) {
                    int nx = x + dx[i];
                    int ny = y + dy[i];
                    
                    if (nx >= 0 && nx < game.board_size && 
                        ny >= 0 && ny < game.board_size && 
                        game.grid[ny][nx] == -1) {
                        count++;
                    }
                }
                
                game.grid[y][x] = count;
            }
        }
    }
}

void reveal_cell(int x, int y) {
    if (x < 0 || x >= game.board_size || y < 0 || y >= game.board_size)
        return;
    if (game.revealed[y][x] || game.flagged[y][x])
        return;

    int queue[MAX_SIZE * MAX_SIZE][2];
    int front = 0, back = 0;

    queue[back][0] = x;
    queue[back][1] = y;
    back++;

    while (front < back) {
        int cx = queue[front][0];
        int cy = queue[front][1];
        front++;

        if (cx < 0 || cx >= game.board_size || cy < 0 || cy >= game.board_size)
            continue;
        if (game.revealed[cy][cx] || game.flagged[cy][cx])
            continue;

        game.revealed[cy][cx] = 1;

        if (game.grid[cy][cx] == -1) {
            game.game_over = 1;
            reveal_all_mines();
            return;
        }

        if (game.grid[cy][cx] == 0) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    queue[back][0] = cx + dx;
                    queue[back][1] = cy + dy;
                    back++;
                }
            }
        }
    }
}


void toggle_flag(int x, int y) {
    if (!game.revealed[y][x]) {
        game.flagged[y][x] = !game.flagged[y][x];
    }
}

void handle_first_click(int x, int y) {
    // Place mines (avoiding the first click position)
    place_mines(x, y);
    
    // Calculate adjacent mine counts for all cells
    calculate_adjacent_mines();
    
    // Now reveal the first cell and potentially adjacent cells
    reveal_cell(x, y);
    game.first_click = 0;
}

int check_win_condition(){
    int hidden_safe_cells = 0;
    for (int y = 0; y < game.board_size; y++) {
        for (int x = 0; x < game.board_size; x++) {
            // Count non-mine cells that are still hidden
            if (!game.revealed[y][x] && game.grid[y][x] != -1) {
                hidden_safe_cells++;
            }   
        }
    }
    return (hidden_safe_cells == 0);
}

void reveal_all_mines() {
    for (int y = 0; y < game.board_size; y++) {
        for (int x = 0; x < game.board_size; x++) {
            if (game.grid[y][x] == -1) { // If it's a mine
                game.revealed[y][x] = 1; // Reveal it
            }
        }
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

void move_cursor(int dx, int dy) {
    int new_x = game.cursor_x + dx;
    int new_y = game.cursor_y + dy;
    
    // Boundary checking
    if (new_x >= 0 && new_x < game.board_size) {
        game.cursor_x = new_x;
    }
    if (new_y >= 0 && new_y < game.board_size) {
        game.cursor_y = new_y;
    }
}

void handle_input(){
    int current_switches = *SWITCHES;
    int current_keys = *keys1;

    static int last_difficulty = -1;
    int current_difficulty = get_level();
    if(current_difficulty != last_difficulty){
        init_game(current_difficulty);
        last_difficulty = current_difficulty;
        draw_current_board(); // Draw board immediatly after init
        return;
    }

    // Checks if key is pressed
    if (current_keys & (1 << KEY_enter) && !(game.last_keys & (1 << KEY_enter))) {
        if (current_switches & (1 << SW_up))
        {
            move_cursor(0, -1);
        }
        if (current_switches & (1 << SW_down))
        {
            move_cursor(0,1);
        }
        if (current_switches & (1 << SW_right))
        {
            move_cursor(1, 0);
        }
        if (current_switches & (1 << SW_left))
        {
            move_cursor(-1, 0);
        }
        if (current_switches & (1 << SW_ACTION_1))
        {
            process_action(SW_ACTION_1);
        }
        if (current_switches & (1 << SW_ACTION_2))
        {
            process_action(SW_ACTION_2);
        }
    }
    game.last_switches = current_switches;
    game.last_keys = current_keys;
}

void process_action(int action) {
    if (game.game_over || game.game_won) {
        return; // No actions allowed if game is over or won
    }

    switch (action) {
        case SW_ACTION_1: // Toggle flag
            toggle_flag(game.cursor_x, game.cursor_y);
            break;
        case SW_ACTION_2: // Reveal
            if (game.first_click) {
                handle_first_click(game.cursor_x, game.cursor_y);
            } else {
                reveal_cell(game.cursor_x, game.cursor_y);
            }
            break;
    }

    if (check_win_condition()) {
        game.game_won = 1;
    }
}

void game_loop() {
    while (1) {
        handle_input();
        
        if (!game.game_over && !game.game_won) {
            draw_current_board(); // Use the new function
        } else {
            clear_screen(game.game_over ? 0x04 : 0x02);
            // Add game over/win message display
        }
        
        // Small delay
        for (volatile int i = 0; i < 10000; i++);
    }
}