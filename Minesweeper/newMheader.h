#ifndef MINESWEEPER_H
#define MINESWEEPER_H

// =====================================================
//                   CONSTANTS
// =====================================================

#define MAX_SIZE        16
#define EASY_SIZE       8
#define MEDIUM_SIZE     12
#define HARD_SIZE       16

#define EASY_MINES      10
#define MEDIUM_MINES    20
#define HARD_MINES      40

#define CELL_SIZE       8
#define VGA_WIDTH       320
#define VGA_HEIGHT      240
#define VGA_BUFFER_SIZE (VGA_WIDTH * VGA_HEIGHT)

// =====================================================
//                 GAME STRUCTURES
// =====================================================

typedef struct {
    int board_size;
    int mine_count;
    int first_click;
    int game_over;
    int game_won;
    int cursor_x;
    int cursor_y;
    int grid[MAX_SIZE][MAX_SIZE];
    int revealed[MAX_SIZE][MAX_SIZE];
    int flagged[MAX_SIZE][MAX_SIZE];
} GameState;

// =====================================================
//                FUNCTION PROTOTYPES
// =====================================================

// VGA
void init_vga(void);
void clear_screen(char color);
void draw_square(int x0, int y0, int width, int height, char color);
void draw_pixel(int x, int y, char color);
void draw_digit(int x, int y, int digit, char color);

// Game logic
void init_game(int difficulty);
void draw_current_board(void);
void draw_cell(int grid_x, int grid_y);
void place_mines(int first_x, int first_y);
void calculate_adjacent_mines(void);
void reveal_all_mines(void);
void reveal_cell(int x, int y);
void toggle_flag(int x, int y);
void handle_first_click(int x, int y);
int  check_win_condition(void);

// Input & main loop
void handle_input(void);
void move_cursor(int dx, int dy);
void process_action(int action);
void game_loop(void);
int  get_level(void); // You might have this elsewhere (difficulty selector)

// Random
void srand_custom(int s);
int  rand(void);

// Global GameState
extern GameState game;

#endif // MINESWEEPER_H
