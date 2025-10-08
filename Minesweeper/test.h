#ifndef TEST.H
#define TEST.H

// Game constants - Adjusted for better gameplay
#define MAX_SIZE 16
#define EASY_SIZE 8
#define MEDIUM_SIZE 12  
#define HARD_SIZE 16
#define EASY_MINES 10
#define MEDIUM_MINES 20
#define HARD_MINES 40

// Game States
typedef struct {
    int grid[MAX_SIZE][MAX_SIZE];      // -1 = mine, 0-8 = adjacent mines
    int revealed[MAX_SIZE][MAX_SIZE];  // 1 = cell is shown, 0 = hidden
    int flagged[MAX_SIZE][MAX_SIZE];   // 1 = cell has flag, 0 = no flag
    int board_size;                    // Current grid size
    int mine_count;                    // Number of mines
    int game_over;                     // 1 = lost, 0 = still playing
    int game_won;                      // 1 = won, 0 = still playing
    int first_click;                   // Special handling for first move
    int cursor_x, cursor_y;            // Player's current position
    int last_switches, last_keys;      // Previous input states
} GameState;

extern GameState game;

// Function declarations
void init_game(int difficulty);
void setup_board(int first_x, int first_y);
int count_adjacent_mines(int x, int y);
void draw_pixel(int x, int y, char color);
void draw_rect(int x, int y, int width, int height, char color);
void draw_block(int x, int y, int width, int height, char color);
void draw_digit(int grid_x, int grid_y, int number, char color);
void draw_cell(int cell_x, int cell_y);
void draw_cursor(void);
void draw_board(void);
void reveal_cell(int x, int y);
void flood_fill(int x, int y);
void flag(int x, int y);
void check_win_condition(void);
int get_level(void);
void move_cursor(int dx, int dy);
void process_action(int action);
void handle_input(void);
void draw_game_over(void);
void delay(int cycles);
void handle_interrupt(void);

// Utility functions
void memory_set(void* ptr, char value, int size);
unsigned int custom_rand(unsigned int* seed);
int custom_strlen(const char* str);

#endif
