#ifndef MINESWEEPER_H
#define MINESWEEPER_H

// Memory Addresses
#define VGA_Buffer 0x8000000
#define VGA_DMA 0x4000100
#define SWITCH_base 0x4000010
#define KEY1_base 0x40000df

# define TIMER_base 0x4000020
# define HEX_base 0x4000050
# define KEY1_base 0x40000df

// Memory Addresses
#define VGA_Buffer 0x8000000
#define VGA_DMA 0x4000100
#define SWITCH_base 0x4000010
#define KEY1_base 0x40000df

// Colors
#define black     0x00
#define white     0xFF
#define red       0xE0
#define green     0x1C
#define blue      0x03
#define yellow    0xFC
#define cyan      0x1F
#define magenta   0xE3
#define gray      0x92
#define dark_gray 0x49
#define light_blue 0x9F

// Game constants
#define MAX_SIZE 20
#define EASY_SIZE 8
#define MEDIUM_SIZE 16
#define HARD_SIZE 20
#define EASY_MINES 10
#define MEDIUM_MINES 35
#define HARD_MINES 80

// Switch and Key Assignments
#define SW_l1 0
#define SW_l2 1
#define SW_l3 2
#define SW_up 3
#define SW_down 4
#define SW_right 5
#define SW_left 6
#define SW_flag 7
#define SW_reveal 8
#define KEY_enter 0

// Function declarations
void init_game(int difficulty);
void setup_board(int first_x, int first_y);
int count_adjacent_mines(int x, int y);
void place_mine(int avoid_x, int avoid_y);
void draw_pixel(int x, int y, char color);
void draw_rect(int x, int y, int width, int height, char color);
void draw_block(int x, int y, int width, int height, char color);
void draw_number(int grid_x, int grid_y, int number);
void draw_cell(int x, int y, int cell_x, int cell_y);
void draw_cursor(void);
void draw_board(void);
void reveal_cell(int x, int y);
void flood_fill(int x, int y);
void flag(int x, int y);
void check_win_condition(void);
int get_level(void);
void move_cursor(int dx, int dy);
void process_action(int action);
void handel_input(void);
void draw_game_over(void);
void delay(int cycles);

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

extern GameState game;

#endif