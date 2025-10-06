#ifndef MINESWEEPER_H
#define MINESWEEPER_H

// Game constants
#define MAX_SIZE 20
#define EASY_SIZE 8
#define MEDIUM_SIZE 16
#define HARD_SIZE 20
#define EASY_MINES 10
#define MEDIUM_MINES 35
#define HARD_MINES 80


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
void handle_input(void);
void draw_game_over(void);
void delay(int cycles);

void handle_interrupt(void);
int my_rand(void);
void my_srand(int seed);

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