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

// Minesweeper Game Implementation
volatile int *VGA_BUFFER = (volatile int*)VGA_Buffer;
volatile int *SWITCHES = (volatile int*)SWITCH_base;
volatile int *KEYS = (volatile int*)KEY1_base;

// Game constants
#define MAX_SIZE 20
#define CELL_SIZE 16
#define BOARD_OFFSET_X 50
#define BOARD_OFFSET_Y 50

// Game states
typedef enum {
    MENU,
    PLAYING,
    GAME_OVER,
    GAME_WON
} GameState;

// Cell states
typedef enum {
    HIDDEN,
    REVEALED,
    FLAGGED
} CellState;

// Difficulty levels
typedef enum {
    EASY,
    MEDIUM,
    HARD
} Difficulty;

// Game structure
typedef struct {
    int board[MAX_SIZE][MAX_SIZE];
    CellState state[MAX_SIZE][MAX_SIZE];
    int width;
    int height;
    int mine_count;
    int cursor_x;
    int cursor_y;
    int flags_placed;
    int mines_flagged;
    GameState game_state;
    Difficulty difficulty;
    int first_move;
    int flag_mode;
} MinesweeperGame;

MinesweeperGame game;

// Function prototypes
void init_game();
void generate_mines();
void calculate_numbers();
void draw_cell(int x, int y, int cell_x, int cell_y);
void draw_board();
void draw_cursor();
void draw_ui();
void reveal_cell(int x, int y);
void toggle_flag(int x, int y);
int count_adjacent_mines(int x, int y);
void flood_fill(int x, int y);
int check_win();
void handle_input();
void vga_draw_rect(int x, int y, int width, int height, int color);
void vga_draw_text(int x, int y, const char *text, int color);
void vga_clear_screen();
int get_switch_state(int switch_num);
int is_key_pressed(int key);

// Simple memory functions (since we can't use stdlib.h)
void memory_set(void *ptr, int value, unsigned int num) {
    unsigned char *p = (unsigned char*)ptr;
    for (unsigned int i = 0; i < num; i++) {
        p[i] = (unsigned char)value;
    }
}

void memory_copy(void *dest, const void *src, unsigned int num) {
    unsigned char *d = (unsigned char*)dest;
    const unsigned char *s = (const unsigned char*)src;
    for (unsigned int i = 0; i < num; i++) {
        d[i] = s[i];
    }
}

// VGA drawing functions
void vga_clear_screen() {
    for (int i = 0; i < 320 * 240; i++) {
        VGA_BUFFER[i] = black;
    }
}

void vga_draw_rect(int x, int y, int width, int height, int color) {
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int pos = (y + dy) * 320 + (x + dx);
            if (pos >= 0 && pos < 320 * 240) {
                VGA_BUFFER[pos] = color;
            }
        }
    }
}

void vga_draw_text(int x, int y, const char *text, int color) {
    // Simple 8x8 font drawing (basic implementation)
    int start_x = x;
    while (*text) {
        char c = *text++;
        // Draw a simple block letter for each character
        for (int dy = 0; dy < 8; dy++) {
            for (int dx = 0; dx < 8; dx++) {
                int pos = (y + dy) * 320 + (x + dx);
                if (pos >= 0 && pos < 320 * 240) {
                    VGA_BUFFER[pos] = (dx > 1 && dx < 6 && dy > 1 && dy < 6) ? color : black;
                }
            }
        }
        x += 9;
    }
}

// Game initialization
void init_game() {
    // Set game parameters based on difficulty
    switch (game.difficulty) {
        case EASY:
            game.width = 8;
            game.height = 8;
            game.mine_count = 10;
            break;
        case MEDIUM:
            game.width = 12;
            game.height = 12;
            game.mine_count = 24;
            break;
        case HARD:
            game.width = 16;
            game.height = 16;
            game.mine_count = 48;
            break;
    }
    
    // Initialize board
    for (int y = 0; y < game.height; y++) {
        for (int x = 0; x < game.width; x++) {
            game.board[x][y] = 0;
            game.state[x][y] = HIDDEN;
        }
    }
    
    game.cursor_x = 0;
    game.cursor_y = 0;
    game.flags_placed = 0;
    game.mines_flagged = 0;
    game.game_state = PLAYING;
    game.first_move = 1;
    game.flag_mode = 0;
}

void generate_mines(int avoid_x, int avoid_y) {
    int mines_placed = 0;
    
    while (mines_placed < game.mine_count) {
        int x = 0, y = 0;
        // Simple pseudo-random placement (using timer or switches for randomness)
        static int seed = 0;
        seed = (seed + 1) % 1000;
        
        x = (seed * 17) % game.width;
        y = (seed * 23) % game.height;
        
        // Avoid the first clicked cell and its neighbors
        if ((abs(x - avoid_x) > 1 || abs(y - avoid_y) > 1) && 
            game.board[x][y] != -1) {
            game.board[x][y] = -1; // -1 represents a mine
            mines_placed++;
        }
    }
}

void calculate_numbers() {
    for (int y = 0; y < game.height; y++) {
        for (int x = 0; x < game.width; x++) {
            if (game.board[x][y] != -1) {
                game.board[x][y] = count_adjacent_mines(x, y);
            }
        }
    }
}

int count_adjacent_mines(int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < game.width && ny >= 0 && ny < game.height) {
                if (game.board[nx][ny] == -1) {
                    count++;
                }
            }
        }
    }
    return count;
}

// Drawing functions
void draw_cell(int x, int y, int cell_x, int cell_y) {
    int screen_x = BOARD_OFFSET_X + cell_x * CELL_SIZE;
    int screen_y = BOARD_OFFSET_Y + cell_y * CELL_SIZE;
    
    // Draw cell background
    int bg_color = light_gray;
    if (game.state[cell_x][cell_y] == REVEALED) {
        bg_color = white;
    } else if (game.state[cell_x][cell_y] == FLAGGED) {
        bg_color = light_red;
    }
    
    vga_draw_rect(screen_x, screen_y, CELL_SIZE, CELL_SIZE, bg_color);
    vga_draw_rect(screen_x, screen_y, CELL_SIZE, 1, dark_gray);
    vga_draw_rect(screen_x, screen_y, 1, CELL_SIZE, dark_gray);
    vga_draw_rect(screen_x + CELL_SIZE - 1, screen_y, 1, CELL_SIZE, dark_gray);
    vga_draw_rect(screen_x, screen_y + CELL_SIZE - 1, CELL_SIZE, 1, dark_gray);
    
    // Draw cell content
    if (game.state[cell_x][cell_y] == REVEALED) {
        if (game.board[cell_x][cell_y] == -1) {
            // Mine
            vga_draw_rect(screen_x + 4, screen_y + 4, CELL_SIZE - 8, CELL_SIZE - 8, black);
        } else if (game.board[cell_x][cell_y] > 0) {
            // Number
            char num_text[2];
            num_text[0] = '0' + game.board[cell_x][cell_y];
            num_text[1] = '\0';
            
            int color = blue;
            switch (game.board[cell_x][cell_y]) {
                case 1: color = blue; break;
                case 2: color = green; break;
                case 3: color = red; break;
                case 4: color = dark_blue; break;
                case 5: color = brown; break;
                case 6: color = cyan; break;
                case 7: color = black; break;
                case 8: color = gray; break;
            }
            
            vga_draw_text(screen_x + 4, screen_y + 4, num_text, color);
        }
    } else if (game.state[cell_x][cell_y] == FLAGGED) {
        // Flag
        vga_draw_rect(screen_x + 4, screen_y + 4, CELL_SIZE - 8, CELL_SIZE - 8, red);
    }
}

void draw_board() {
    for (int y = 0; y < game.height; y++) {
        for (int x = 0; x < game.width; x++) {
            draw_cell(x, y, x, y);
        }
    }
}

void draw_cursor() {
    int screen_x = BOARD_OFFSET_X + game.cursor_x * CELL_SIZE;
    int screen_y = BOARD_OFFSET_Y + game.cursor_y * CELL_SIZE;
    
    // Draw cursor (yellow border)
    vga_draw_rect(screen_x, screen_y, CELL_SIZE, 2, yellow);
    vga_draw_rect(screen_x, screen_y, 2, CELL_SIZE, yellow);
    vga_draw_rect(screen_x + CELL_SIZE - 2, screen_y, 2, CELL_SIZE, yellow);
    vga_draw_rect(screen_x, screen_y + CELL_SIZE - 2, CELL_SIZE, 2, yellow);
}

void draw_ui() {
    // Draw game info
    char info_text[50];
    
    // Difficulty info
    const char *diff_text = "";
    switch (game.difficulty) {
        case EASY: diff_text = "EASY"; break;
        case MEDIUM: diff_text = "MEDIUM"; break;
        case HARD: diff_text = "HARD"; break;
    }
    
    vga_draw_text(10, 10, "MINESWEEPER", white);
    vga_draw_text(10, 25, "Difficulty: ", white);
    vga_draw_text(100, 25, diff_text, yellow);
    
    // Mines info
    int mines_left = game.mine_count - game.flags_placed;
    char mines_text[20];
    char *mines_ptr = mines_text;
    int mines_temp = mines_left;
    
    // Convert number to string
    if (mines_temp == 0) {
        *mines_ptr++ = '0';
    } else {
        char buffer[10];
        int i = 0;
        while (mines_temp > 0) {
            buffer[i++] = '0' + (mines_temp % 10);
            mines_temp /= 10;
        }
        while (i > 0) {
            *mines_ptr++ = buffer[--i];
        }
    }
    *mines_ptr = '\0';
    
    vga_draw_text(10, 40, "Mines left: ", white);
    vga_draw_text(100, 40, mines_text, red);
    
    // Flag mode indicator
    if (game.flag_mode) {
        vga_draw_text(200, 10, "FLAG MODE", red);
    } else {
        vga_draw_text(200, 10, "REVEAL MODE", green);
    }
    
    // Game state messages
    if (game.game_state == GAME_OVER) {
        vga_draw_text(100, 200, "GAME OVER!", red);
        vga_draw_text(80, 215, "Press enter to restart", yellow);
    } else if (game.game_state == GAME_WON) {
        vga_draw_text(100, 200, "YOU WIN!", green);
        vga_draw_text(80, 215, "Press enter to restart", yellow);
    }
}

// Game logic functions
void reveal_cell(int x, int y) {
    if (x < 0 || x >= game.width || y < 0 || y >= game.height) return;
    if (game.state[x][y] != HIDDEN) return;
    
    // Generate mines on first move (avoiding clicked cell and neighbors)
    if (game.first_move) {
        generate_mines(x, y);
        calculate_numbers();
        game.first_move = 0;
    }
    
    game.state[x][y] = REVEALED;
    
    if (game.board[x][y] == -1) {
        game.game_state = GAME_OVER;
        // Reveal all mines
        for (int cy = 0; cy < game.height; cy++) {
            for (int cx = 0; cx < game.width; cx++) {
                if (game.board[cx][cy] == -1) {
                    game.state[cx][cy] = REVEALED;
                }
            }
        }
    } else if (game.board[x][y] == 0) {
        // Flood fill for empty cells
        flood_fill(x, y);
    }
}

void toggle_flag(int x, int y) {
    if (x < 0 || x >= game.width || y < 0 || y >= game.height) return;
    if (game.state[x][y] == HIDDEN) {
        game.state[x][y] = FLAGGED;
        game.flags_placed++;
        if (game.board[x][y] == -1) {
            game.mines_flagged++;
        }
    } else if (game.state[x][y] == FLAGGED) {
        game.state[x][y] = HIDDEN;
        game.flags_placed--;
        if (game.board[x][y] == -1) {
            game.mines_flagged--;
        }
    }
}

void flood_fill(int x, int y) {
    if (x < 0 || x >= game.width || y < 0 || y >= game.height) return;
    if (game.state[x][y] != HIDDEN) return;
    if (game.board[x][y] == -1) return;
    
    game.state[x][y] = REVEALED;
    
    if (game.board[x][y] == 0) {
        // Recursively reveal adjacent cells
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx != 0 || dy != 0) {
                    flood_fill(x + dx, y + dy);
                }
            }
        }
    }
}

int check_win() {
    int revealed_count = 0;
    int total_safe_cells = game.width * game.height - game.mine_count;
    
    for (int y = 0; y < game.height; y++) {
        for (int x = 0; x < game.width; x++) {
            if (game.state[x][y] == REVEALED && game.board[x][y] != -1) {
                revealed_count++;
            }
        }
    }
    
    return revealed_count == total_safe_cells;
}

// Input handling
int get_switch_state(int switch_num) {
    return (*SWITCHES >> switch_num) & 1;
}

int is_key_pressed(int key) {
    return (*KEYS >> key) & 1;
}

void handle_input() {
    static int last_enter_state = 0;
    static int last_up_state = 0;
    static int last_down_state = 0;
    static int last_left_state = 0;
    static int last_right_state = 0;
    static int last_action1_state = 0;
    static int last_action2_state = 0;
    
    int current_enter = is_key_pressed(KEY_enter);
    int current_up = get_switch_state(SW_up);
    int current_down = get_switch_state(SW_down);
    int current_left = get_switch_state(SW_left);
    int current_right = get_switch_state(SW_right);
    int current_action1 = get_switch_state(SW_ACTION_1);
    int current_action2 = get_switch_state(SW_ACTION_2);
    
    // Handle menu/difficulty selection
    if (game.game_state == MENU) {
        if (get_switch_state(SW_l1)) game.difficulty = EASY;
        else if (get_switch_state(SW_l2)) game.difficulty = MEDIUM;
        else if (get_switch_state(SW_l3)) game.difficulty = HARD;
        
        if (current_enter && !last_enter_state) {
            init_game();
            game.game_state = PLAYING;
        }
    }
    // Handle gameplay
    else if (game.game_state == PLAYING) {
        // Cursor movement with double-press detection
        if (current_up && !last_up_state) {
            if (game.cursor_y > 0) game.cursor_y--;
        }
        if (current_down && !last_down_state) {
            if (game.cursor_y < game.height - 1) game.cursor_y++;
        }
        if (current_left && !last_left_state) {
            if (game.cursor_x > 0) game.cursor_x--;
        }
        if (current_right && !last_right_state) {
            if (game.cursor_x < game.width - 1) game.cursor_x++;
        }
        
        // Toggle flag mode
        if (current_action1 && !last_action1_state) {
            game.flag_mode = !game.flag_mode;
        }
        
        // Cell actions
        if (current_enter && !last_enter_state) {
            if (game.flag_mode) {
                toggle_flag(game.cursor_x, game.cursor_y);
                if (check_win()) {
                    game.game_state = GAME_WON;
                }
            } else {
                reveal_cell(game.cursor_x, game.cursor_y);
                if (game.game_state != GAME_OVER && check_win()) {
                    game.game_state = GAME_WON;
                }
            }
        }
    }
    // Handle game over/win states
    else if ((game.game_state == GAME_OVER || game.game_state == GAME_WON) && 
             current_enter && !last_enter_state) {
        game.game_state = MENU;
    }
    
    // Update last states
    last_enter_state = current_enter;
    last_up_state = current_up;
    last_down_state = current_down;
    last_left_state = current_left;
    last_right_state = current_right;
    last_action1_state = current_action1;
    last_action2_state = current_action2;
}

// Main game loop
int main() {
    // Initialize game
    game.difficulty = EASY;
    game.game_state = MENU;
    
    while (1) {
        vga_clear_screen();
        handle_input();
        
        if (game.game_state == MENU) {
            // Draw menu
            vga_draw_text(100, 80, "MINESWEEPER", white);
            vga_draw_text(90, 110, "Select Difficulty:", yellow);
            vga_draw_text(100, 130, "SW1: Easy", game.difficulty == EASY ? green : white);
            vga_draw_text(100, 150, "SW2: Medium", game.difficulty == MEDIUM ? green : white);
            vga_draw_text(100, 170, "SW3: Hard", game.difficulty == HARD ? green : white);
            vga_draw_text(80, 200, "Press ENTER to start", cyan);
        } else {
            draw_board();
            draw_cursor();
            draw_ui();
        }
        
        // Simple delay for debouncing
        for (volatile int i = 0; i < 10000; i++);
    }
    
    return 0;
}