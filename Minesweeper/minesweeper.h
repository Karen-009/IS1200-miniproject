#ifndef MINESWEEPER_H
#define MINESWEEPER_H

// Game constants
#define MAX_SIZE 20
#define EASY_SIZE 6
#define MEDIUM_SIZE 8
#define HARD_SIZE 10
#define EASY_MINES 15
#define MEDIUM_MINES 20
#define HARD_MINES 60
#define CELL_SIZE 20


// Function declarations
void handle_interrupt();

// Game initialization
void init_game(int dificulty);
void init_vga(void);

// VGA / Graphics functions
void draw_pixel(int x, int y, char color);
void clear_screen(char color);
void draw_square(int x0, int y0, int width, int height, char color);
void draw_current_board();
void draw_digit(int x, int y, int digit, char color);
void draw_cell(int grid_x, int grid_y);

// Mines and game logic
void place_mines(int first_x, int first_y);
void srand_custom(int s);
int rand();
void calculate_adjacent_mines();
void reveal_cell(int x, int y);
void toggle_flag(int x, int y);
void handle_first_click(int x, int y);
int check_win_condition();
void reveal_all_mines();

// Input and cursor handling
int get_level();
void move_cursor(int dx, int dy);
void handle_input();
void process_action(int action);

// Main game loop
void game_loop();


//Game States
typedef struct {
    int first_click;
    int game_over;
    int game_won;
    int cursor_x;
    int cursor_y;
    int last_switches;
    int last_keys;
    int board_size;
    int mine_count;
    int grid[MAX_SIZE][MAX_SIZE];      // -1 = mine, 0-8 = adjacent mines
    int revealed[MAX_SIZE][MAX_SIZE];  // 0 = hidden, 1 = revealed
    int flagged[MAX_SIZE][MAX_SIZE];   // 0 = not flagged, 1 = flagged
} GameState;

extern GameState game;

#endif
