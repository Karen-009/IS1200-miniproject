// Board definitions
#ifndef DTEKV_BOARD_H
#define DTEKV_BOARD_H

// Memory Addresses for VGA
#define VGA_Buffer 0x8000000
#define VGA_DMA 0x4000100
#define SWITCH_base 0x4000010
#define KEY1_base 0x40000d0
#define TIMER_base 0x4000020
#define HEX_base 0x4000050
#define SWITCH_BASE 0x4000010  

// Colors 
#define black     0x00
#define white     0xFF
#define red       0xE0
#define yellow    0xFC
#define green     0x1C
#define blue      0x03
#define cyan      0x1F
#define magenta   0xE3
#define gray      0x92
#define dark_gray 0x49
#define light_blue 0x9F
#define orange    0xFC 
#define light_gray 0xE4 
#define light_green 0x7C
#define light_red 0xF4
#define light_yellow 0xFE
#define purple    0xA3
#define brown     0xB2
#define pink      0xF3
#define light_cyan 0xBF
#define light_magenta 0xF3
#define dark_blue 0x02
#define dark_green 0x0C
#define pastel_pink 0xF5 

// Switch and Key Assignments
#define SW_l1 1     // easy difficulty
#define SW_l2 2   // medium difficulty
#define SW_l3 3   // hard  difficulty
#define SW_up 4   // move cursor up
#define SW_down 5   // move cursor down
#define SW_right 6  // move cursor right
#define SW_left 7      // move cursor left
#define SW_ACTION_1 8   // toggle flag mode/ erase cell
#define SW_ACTION_2 9 // reveal cell/ enter digit mode
#define KEY_enter 0 // confirm action

#endif

// Game constants
#define EASY_WIDTH 8
#define EASY_HEIGHT 8
#define EASY_MINES 10

#define MEDIUM_WIDTH 12
#define MEDIUM_HEIGHT 12  
#define MEDIUM_MINES 20

#define HARD_WIDTH 16
#define HARD_HEIGHT 16
#define HARD_MINES 40

#define CELL_SIZE 20
#define BORDER 50
#define STATUS_HEIGHT 40
#define MAX_WIDTH 16
#define MAX_HEIGHT 16

// Game states
typedef enum {
    MENU,
    PLAYING,
    WIN,
    LOSE
} GameState;

// Cell states
typedef struct {
    int is_mine;
    int is_revealed;
    int is_flagged;
    int adjacent_mines;
} Cell;

// Game structure
typedef struct {
    Cell board[MAX_HEIGHT][MAX_WIDTH];
    int width;
    int height;
    int total_mines;
    int flags_placed;
    int cells_revealed;
    int cursor_x;
    int cursor_y;
    GameState state;
    int game_started;
} Game;

// Global variables
volatile uint8_t* vga_buffer = (volatile uint8_t*)VGA_Buffer;
volatile uint32_t* switches = (volatile uint32_t*)SWITCH_base;
volatile uint32_t* keys = (volatile uint32_t*)KEY1_base;
volatile uint32_t* timer = (volatile uint32_t*)TIMER_base;

Game game;
int last_key_state = 0;
int move_switch_active = 0;

// Custom utility functions
void memory_set(void* ptr, uint8_t value, int size) {
    uint8_t* p = (uint8_t*)ptr;
    for (int i = 0; i < size; i++) {
        p[i] = value;
    }
}

void memory_copy(void* dest, const void* src, int size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (int i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

int custom_strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void int_to_string(int num, char* buffer) {
    int i = 0;
    int is_negative = 0;
    
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    // Handle zero case
    if (num == 0) {
        buffer[i++] = '0';
    } else {
        // Extract digits
        while (num > 0) {
            buffer[i++] = '0' + (num % 10);
            num /= 10;
        }
    }
    
    // Add negative sign if needed
    if (is_negative) {
        buffer[i++] = '-';
    }
    
    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
    
    buffer[i] = '\0';
}

// Simple random number generator (linear congruential generator)
unsigned int custom_rand(unsigned int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

// Function prototypes
void initialize_game(int difficulty);
void generate_mines();
void calculate_adjacent_mines();
void reveal_cell(int x, int y);
void reveal_all_mines();
int count_adjacent_flags(int x, int y);
void chord_cell(int x, int y);
void draw_cell(int x, int y, int screen_x, int screen_y);
void draw_board();
void draw_menu();
void draw_status();
void handle_input();

// Memory-mapped I/O functions
void write_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < 640 && y >= 0 && y < 480) {
        vga_buffer[y * 640 + x] = color;
    }
}

void draw_rect(int x, int y, int width, int height, uint8_t color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            write_pixel(x + j, y + i, color);
        }
    }
}

// Simple 8x8 font data for digits and basic characters
const uint8_t font_data[10][8] = {
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // 0
    {0x0C, 0x1C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // 1
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // 2
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // 3
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // 4
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // 5
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // 6
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // 7
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // 8
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}  // 9
};

void draw_char(int x, int y, char c, uint8_t color) {
    // Only handle digits for now
    if (c >= '0' && c <= '9') {
        int digit = c - '0';
        for (int i = 0; i < 8; i++) {
            uint8_t row = font_data[digit][i];
            for (int j = 0; j < 8; j++) {
                if (row & (1 << (7 - j))) {
                    write_pixel(x + j, y + i, color);
                }
            }
        }
    }
}

void draw_string(int x, int y, const char* str, uint8_t color) {
    int len = custom_strlen(str);
    for (int i = 0; i < len; i++) {
        draw_char(x + i * 8, y, str[i], color);
    }
}

void clear_screen(uint8_t color) {
    for (int y = 0; y < 480; y++) {
        for (int x = 0; x < 640; x++) {
            write_pixel(x, y, color);
        }
    }
}

uint32_t read_switches() {
    return *switches;
}

uint32_t read_keys() {
    return *keys;
}

uint32_t read_timer() {
    return *timer;
}

// Game functions
void initialize_game(int difficulty) {
    // Reset game board
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            game.board[i][j].is_mine = 0;
            game.board[i][j].is_revealed = 0;
            game.board[i][j].is_flagged = 0;
            game.board[i][j].adjacent_mines = 0;
        }
    }
    
    // Set game parameters based on difficulty
    switch (difficulty) {
        case 1: // Easy
            game.width = EASY_WIDTH;
            game.height = EASY_HEIGHT;
            game.total_mines = EASY_MINES;
            break;
        case 2: // Medium
            game.width = MEDIUM_WIDTH;
            game.height = MEDIUM_HEIGHT;
            game.total_mines = MEDIUM_MINES;
            break;
        case 3: // Hard
            game.width = HARD_WIDTH;
            game.height = HARD_HEIGHT;
            game.total_mines = HARD_MINES;
            break;
        default:
            game.width = EASY_WIDTH;
            game.height = EASY_HEIGHT;
            game.total_mines = EASY_MINES;
    }
    
    // Initialize game state
    game.flags_placed = 0;
    game.cells_revealed = 0;
    game.cursor_x = 0;
    game.cursor_y = 0;
    game.state = PLAYING;
    game.game_started = 0;
}

void generate_mines() {
    int mines_placed = 0;
    unsigned int seed = read_timer(); // Use timer as random seed
    
    while (mines_placed < game.total_mines) {
        int x = custom_rand(&seed) % game.width;
        int y = custom_rand(&seed) % game.height;
        
        // Don't place mine at cursor position on first click
        if (!game.game_started && x == game.cursor_x && y == game.cursor_y) {
            continue;
        }
        
        if (!game.board[y][x].is_mine) {
            game.board[y][x].is_mine = 1;
            mines_placed++;
        }
    }
    
    calculate_adjacent_mines();
    game.game_started = 1;
}

void calculate_adjacent_mines() {
    for (int y = 0; y < game.height; y++) {
        for (int x = 0; x < game.width; x++) {
            if (game.board[y][x].is_mine) {
                game.board[y][x].adjacent_mines = 0;
                continue;
            }
            
            int count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < game.width && ny >= 0 && ny < game.height) {
                        if (game.board[ny][nx].is_mine) {
                            count++;
                        }
                    }
                }
            }
            game.board[y][x].adjacent_mines = count;
        }
    }
}

void reveal_cell(int x, int y) {
    if (x < 0 || x >= game.width || y < 0 || y >= game.height) return;
    if (game.board[y][x].is_revealed || game.board[y][x].is_flagged) return;
    
    game.board[y][x].is_revealed = 1;
    game.cells_revealed++;
    
    if (game.board[y][x].is_mine) {
        game.state = LOSE;
        reveal_all_mines();
        return;
    }
    
    // If cell has no adjacent mines, reveal neighbors
    if (game.board[y][x].adjacent_mines == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                reveal_cell(x + dx, y + dy);
            }
        }
    }
    
    // Check for win condition
    if (game.cells_revealed == (game.width * game.height - game.total_mines)) {
        game.state = WIN;
    }
}

void reveal_all_mines() {
    for (int y = 0; y < game.height; y++) {
        for (int x = 0; x < game.width; x++) {
            if (game.board[y][x].is_mine) {
                game.board[y][x].is_revealed = 1;
            }
        }
    }
}

int count_adjacent_flags(int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < game.width && ny >= 0 && ny < game.height) {
                if (game.board[ny][nx].is_flagged) {
                    count++;
                }
            }
        }
    }
    return count;
}

void chord_cell(int x, int y) {
    if (!game.board[y][x].is_revealed || game.board[y][x].adjacent_mines == 0) return;
    
    int flag_count = count_adjacent_flags(x, y);
    if (flag_count == game.board[y][x].adjacent_mines) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && nx < game.width && ny >= 0 && ny < game.height) {
                    if (!game.board[ny][nx].is_flagged && !game.board[ny][nx].is_revealed) {
                        reveal_cell(nx, ny);
                    }
                }
            }
        }
    }
}

void draw_cell(int x, int y, int screen_x, int screen_y) {
    Cell cell = game.board[y][x];
    uint8_t bg_color = light_gray;
    uint8_t border_color = dark_gray;
    uint8_t text_color = black;
    
    // Draw cell background
    if (x == game.cursor_x && y == game.cursor_y) {
        bg_color = light_blue; // Highlight cursor
    }
    
    if (cell.is_revealed) {
        if (cell.is_mine) {
            bg_color = light_red;
        } else {
            bg_color = white;
        }
    }
    
    draw_rect(screen_x, screen_y, CELL_SIZE, CELL_SIZE, bg_color);
    
    // Draw cell border
    draw_rect(screen_x, screen_y, CELL_SIZE, 1, border_color);
    draw_rect(screen_x, screen_y, 1, CELL_SIZE, border_color);
    draw_rect(screen_x + CELL_SIZE - 1, screen_y, 1, CELL_SIZE, border_color);
    draw_rect(screen_x, screen_y + CELL_SIZE - 1, CELL_SIZE, 1, border_color);
    
    // Draw cell content
    if (cell.is_revealed) {
        if (cell.is_mine) {
            // Draw mine
            draw_rect(screen_x + CELL_SIZE/2 - 2, screen_y + CELL_SIZE/2 - 2, 4, 4, black);
        } else if (cell.adjacent_mines > 0) {
            // Draw number
            char num_str[2];
            num_str[0] = '0' + cell.adjacent_mines;
            num_str[1] = '\0';
            
            // Set color based on number
            switch (cell.adjacent_mines) {
                case 1: text_color = blue; break;
                case 2: text_color = green; break;
                case 3: text_color = red; break;
                case 4: text_color = dark_blue; break;
                case 5: text_color = brown; break;
                case 6: text_color = cyan; break;
                case 7: text_color = black; break;
                case 8: text_color = dark_gray; break;
            }
            
            draw_string(screen_x + CELL_SIZE/2 - 4, screen_y + CELL_SIZE/2 - 4, num_str, text_color);
        }
    } else if (cell.is_flagged) {
        // Draw flag
        draw_rect(screen_x + CELL_SIZE/2 - 2, screen_y + 4, 4, 8, red);
    }
}

void draw_board() {
    int board_width = game.width * CELL_SIZE;
    int board_height = game.height * CELL_SIZE;
    int start_x = (640 - board_width) / 2;
    int start_y = BORDER + STATUS_HEIGHT;
    
    // Draw board background
    draw_rect(start_x - 2, start_y - 2, board_width + 4, board_height + 4, dark_gray);
    
    // Draw cells
    for (int y = 0; y < game.height; y++) {
        for (int x = 0; x < game.width; x++) {
            int screen_x = start_x + x * CELL_SIZE;
            int screen_y = start_y + y * CELL_SIZE;
            draw_cell(x, y, screen_x, screen_y);
        }
    }
}

void draw_menu() {
    clear_screen(blue);
    
    // Draw menu text using simple rectangles (since we don't have full font)
    draw_string(250, 100, "MINESWEEPER", white);
    draw_string(200, 150, "SELECT LEVEL:", white);
    
    // Draw difficulty options with highlighting
    if (read_switches() & (1 << SW_l1)) {
        draw_rect(240, 190, 160, 25, yellow);
        draw_string(250, 200, "EASY 8X8 10MINES", black);
    } else {
        draw_rect(240, 190, 160, 25, white);
        draw_string(250, 200, "EASY 8X8 10MINES", black);
    }
    
    if (read_switches() & (1 << SW_l2)) {
        draw_rect(240, 220, 160, 25, yellow);
        draw_string(250, 230, "MEDIUM 12X12 20MINES", black);
    } else {
        draw_rect(240, 220, 160, 25, white);
        draw_string(250, 230, "MEDIUM 12X12 20MINES", black);
    }
    
    if (read_switches() & (1 << SW_l3)) {
        draw_rect(240, 250, 160, 25, yellow);
        draw_string(250, 260, "HARD 16X16 40MINES", black);
    } else {
        draw_rect(240, 250, 160, 25, white);
        draw_string(250, 260, "HARD 16X16 40MINES", black);
    }
    
    draw_string(220, 320, "PRESS KEY0 TO START", white);
}

void draw_status() {
    int start_y = BORDER;
    
    // Draw background
    draw_rect(0, 0, 640, BORDER + STATUS_HEIGHT, dark_blue);
    
    // Draw game info
    char status[30];
    char mines_str[10];
    char flags_str[10];
    
    int_to_string(game.total_mines - game.flags_placed, mines_str);
    int_to_string(game.total_mines, flags_str);
    
    if (game.state == PLAYING) {
        // Build status string manually
        char status[] = "MINES: ";
        draw_string(20, 20, status, white);
        draw_string(20 + 7*8, 20, mines_str, white);
        draw_string(20 + 7*8 + custom_strlen(mines_str)*8, 20, "/", white);
        draw_string(20 + 7*8 + custom_strlen(mines_str)*8 + 8, 20, flags_str, white);
    } else if (game.state == WIN) {
        draw_string(20, 20, "YOU WIN! PRESS KEY0 FOR MENU", yellow);
    } else if (game.state == LOSE) {
        draw_string(20, 20, "GAME OVER! PRESS KEY0 FOR MENU", red);
    }
    
    // Draw controls info
    draw_string(400, 20, "SW8:FLAG SW9:REVEAL", white);
}

void handle_input() {
    uint32_t current_switches = read_switches();
    uint32_t current_keys = read_keys();
    
    // Check for key press (edge detection)
    int key_pressed = (current_keys & (1 << KEY_enter)) && !(last_key_state & (1 << KEY_enter));
    last_key_state = current_keys;
    
    // Check if move switch is active
    move_switch_active = (current_switches & ((1 << SW_up) | (1 << SW_down) | (1 << SW_left) | (1 << SW_right))) != 0;
    
    if (game.state == MENU) {
        if (key_pressed) {
            if (current_switches & (1 << SW_l1)) {
                initialize_game(1);
            } else if (current_switches & (1 << SW_l2)) {
                initialize_game(2);
            } else if (current_switches & (1 << SW_l3)) {
                initialize_game(3);
            }
        }
        return;
    }
    
    if (game.state == WIN || game.state == LOSE) {
        if (key_pressed) {
            game.state = MENU;
        }
        return;
    }
    
    // Handle movement with switch-based control
    if (key_pressed && move_switch_active) {
        int move_count = 1;
        
        // Check for double movement (switch continuously on)
        static int last_move_time = 0;
        static int last_direction = 0;
        int current_time = read_timer();
        int current_direction = current_switches & ((1 << SW_up) | (1 << SW_down) | (1 << SW_left) | (1 << SW_right));
        
        if (current_direction == last_direction && (current_time - last_move_time) < 1000000) {
            move_count = 2; // Double move if same direction pressed quickly
        }
        
        last_move_time = current_time;
        last_direction = current_direction;
        
        for (int i = 0; i < move_count; i++) {
            if (current_switches & (1 << SW_up)) {
                if (game.cursor_y > 0) game.cursor_y--;
            }
            if (current_switches & (1 << SW_down)) {
                if (game.cursor_y < game.height - 1) game.cursor_y++;
            }
            if (current_switches & (1 << SW_left)) {
                if (game.cursor_x > 0) game.cursor_x--;
            }
            if (current_switches & (1 << SW_right)) {
                if (game.cursor_x < game.width - 1) game.cursor_x++;
            }
        }
    }
    
    // Handle actions
    if (key_pressed && !move_switch_active) {
        if (!game.game_started) {
            generate_mines();
        }
        
        if (current_switches & (1 << SW_ACTION_1)) {
            // Toggle flag
            Cell* cell = &game.board[game.cursor_y][game.cursor_x];
            if (!cell->is_revealed) {
                cell->is_flagged = !cell->is_flagged;
                if (cell->is_flagged) {
                    game.flags_placed++;
                } else {
                    game.flags_placed--;
                }
            }
        } else if (current_switches & (1 << SW_ACTION_2)) {
            // Reveal cell or chord
            Cell* cell = &game.board[game.cursor_y][game.cursor_x];
            if (cell->is_revealed) {
                chord_cell(game.cursor_x, game.cursor_y);
            } else if (!cell->is_flagged) {
                reveal_cell(game.cursor_x, game.cursor_y);
            }
        }
    }
}

int main() {
    // Initialize game
    memory_set(&game, 0, sizeof(Game));
    game.state = MENU;
    
    // Main game loop
    while (1) {
        handle_input();
        
        if (game.state == MENU) {
            draw_menu();
        } else {
            clear_screen(black);
            draw_status();
            draw_board();
        }
        
        // Small delay to prevent flickering
        for (volatile int i = 0; i < 10000; i++);
    }
    
    return 0;
}