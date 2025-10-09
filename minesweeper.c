#include <stdint.h>
#include <stddef.h>
#include "dtekv_board.h"
#include "main_menu.h"
#include "minesweeper.h"
#include "sudoku_vga.h"

extern int menu_state;
extern void draw_text(int x, int y, const char *text, uint8_t color);

uint8_t mine_grid[GRID_MAX_ROWS][GRID_MAX_COLS];
uint8_t adj[GRID_MAX_ROWS][GRID_MAX_COLS]; 
uint8_t state_grid[GRID_MAX_ROWS][GRID_MAX_COLS];
int g_rows = 0, g_cols = 0, g_mines = 0;
int revealed_count = 0;
int game_over = 0;
int cursor_r = 0, cursor_c = 0;

// Memory mapped addresses
#define SW_REG  ((volatile uint32_t*) SWITCH_BASE)
#define KEY_REG ((volatile uint32_t*) KEY1_base)
#define VGA_FB  ((volatile uint8_t*) VGA_Buffer)
#define SCREEN_W 320
#define SCREEN_H 240

#define SW_MASK(x) (1u << (x))

static const LevelSpec LEVELS[3] = {
    {9, 9, 10}, //Easy
    {16, 12, 30}, //Medium
    {24, 16, 70} //Hard
};


static int first_move = 1; 

// Simple LFSR PRNG for embedded
static uint32_t lfsr = 0xACE1u;

uint32_t rand32(void) {
    uint32_t bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1u;
    lfsr = (lfsr >> 1) | (bit << 15);
    uint32_t r = lfsr;
    r ^= (r << 5);
    r ^= (r >> 11);
    r ^= (r << 7);
    return r;
}


void busy_wait(volatile int n) {
    while (n-- > 0) {
        asm volatile("nop");
    }
}

inline void put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
    VGA_FB[y * SCREEN_W + x] = color;
}

void fill_rect(int x0, int y0, int w, int h, uint8_t color) {
    if (w <= 0 || h <= 0) return;
    int x1 = x0 + w, y1 = y0 + h;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > SCREEN_W) x1 = SCREEN_W;
    if (y1 > SCREEN_H) y1 = SCREEN_H;
    for (int y = y0; y < y1; ++y) {
        uint8_t *row = (uint8_t*)&VGA_FB[y * SCREEN_W];
        for (int x = x0; x < x1; ++x) row[x] = color;
    }
}

static const uint8_t font5x7_digits[10][5] = {
    {0x7E,0x81,0x81,0x81,0x7E}, // 0
    {0x00,0x82,0xFF,0x80,0x00}, // 1 
    {0xE2,0x91,0x91,0x91,0x8E}, // 2
    {0x42,0x81,0x89,0x89,0x76}, // 3
    {0x18,0x14,0x12,0xFF,0x10}, // 4
    {0x4F,0x89,0x89,0x89,0x71}, // 5
    {0x7E,0x89,0x89,0x89,0x72}, // 6
    {0x01,0x01,0xF1,0x09,0x07}, // 7
    {0x76,0x89,0x89,0x89,0x76}, // 8
    {0x46,0x89,0x89,0x89,0x7E}  // 9
};

void draw_digit_in_cell(int grid_r, int grid_c, int digit, uint8_t color) {
    if (digit < 0 || digit > 9) return;
    int cell_x = grid_c * CELL_SIZE;
    int cell_y = grid_r * CELL_SIZE;
    /* digit area: 5x7 */
    int gx = cell_x + (CELL_SIZE - 5) / 2;
    int gy = cell_y + (CELL_SIZE - 7) / 2;
    const uint8_t *glyph = font5x7_digits[digit];
    for (int col = 0; col < 5; ++col) {
        uint8_t colbits = glyph[col];
        for (int row = 0; row < 7; ++row) {
            if (colbits & (1 << row)) {
                put_pixel(gx + col, gy + row, color);
            }
        }
    }
}

// Draw cell border
void draw_cell_border(int r, int c, uint8_t border_color) {
    int x0 = c * CELL_SIZE;
    int y0 = r * CELL_SIZE;

    for (int x = x0; x < x0 + CELL_SIZE; ++x) {
        put_pixel(x, y0, border_color);
        put_pixel(x, y0 + CELL_SIZE - 1, border_color);
    }
    for (int y = y0; y < y0 + CELL_SIZE; ++y) {
        put_pixel(x0, y, border_color);
        put_pixel(x0 + CELL_SIZE - 1, y, border_color);
    }
}

// Render whole board
void render_board(void) {
    // Background
    fill_rect(0, 0, SCREEN_W, SCREEN_H, dark_blue);

    // Draw cell
    for (int r = 0; r < g_rows; ++r) {
        for (int c = 0; c < g_cols; ++c) {
            int x0 = c * CELL_SIZE;
            int y0 = r * CELL_SIZE;
            // Cell interior
            if (state_grid[r][c] == HIDDEN) {
                fill_rect(x0 + 1, y0 + 1, CELL_SIZE - 2, CELL_SIZE - 2, dark_gray);
            } else if (state_grid[r][c] == FLAGGED) {
                fill_rect(x0 + 1, y0 + 1, CELL_SIZE - 2, CELL_SIZE - 2, red);
                // Flagg
                int fx = x0 + (CELL_SIZE - 3) / 2;
                int fy = y0 + (CELL_SIZE - 5) / 2;
                fill_rect(fx, fy, 1, 5, white);
                fill_rect(fx+1, fy, 2, 3, yellow);
            } else if (state_grid[r][c] == REVEALED) {
                if (mine_grid[r][c]) {
                    fill_rect(x0 + 1, y0 + 1, CELL_SIZE - 2, CELL_SIZE - 2, light_red);
                    //Mine
                    int cx = x0 + CELL_SIZE/2;
                    int cy = y0 + CELL_SIZE/2;
                    put_pixel(cx, cy, black);
                    put_pixel(cx-1, cy, black);
                    put_pixel(cx+1, cy, black);
                    put_pixel(cx, cy-1, black);
                    put_pixel(cx, cy+1, black);
                } else {
                    fill_rect(x0 + 1, y0 + 1, CELL_SIZE - 2, CELL_SIZE - 2, light_gray);
                    if (adj[r][c] > 0) {
                        //Number color
                        uint8_t col = blue;
                        switch (adj[r][c]) {
                            case 1: col = blue; break;
                            case 2: col = green; break;
                            case 3: col = red; break;
                            case 4: col = dark_blue; break;
                            case 5: col = magenta; break;
                            case 6: col = cyan; break;
                            case 7: col = brown; break;
                            default: col = black; break;
                        }
                        draw_digit_in_cell(r, c, adj[r][c], col);
                    }
                }
            }
            draw_cell_border(r, c, black);
        }
    }

    // Raw cursor (inverted border)
    int cx0 = cursor_c * CELL_SIZE;
    int cy0 = cursor_r * CELL_SIZE;
    // Draw a highlighted border
    for (int x = cx0; x < cx0 + CELL_SIZE; ++x) {
        put_pixel(x, cy0, light_yellow);
        put_pixel(x, cy0 + CELL_SIZE - 1, light_yellow);
    }
    for (int y = cy0; y < cy0 + CELL_SIZE; ++y) {
        put_pixel(cx0, y, light_yellow);
        put_pixel(cx0 + CELL_SIZE - 1, y, light_yellow);
    }
}

// Initialize board arrays
void clear_board_state(void) {
    for (int r = 0; r < GRID_MAX_ROWS; ++r) {
        for (int c = 0; c < GRID_MAX_COLS; ++c) {
            mine_grid[r][c] = 0;
            adj[r][c] = 0;
            state_grid[r][c] = HIDDEN;
        }
    }
}

// Place mines randomly
void place_mines(int rows, int cols, int mines, int safe_r, int safe_c) {
    int placed = 0;
    mine_grid[safe_r][safe_c] = 0; // Ensure first click is not a mine

    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            int rr = safe_r + dr, cc = safe_c + dc;
            if (rr >= 0 && rr < rows && cc >= 0 && cc < cols) {
                mine_grid[rr][cc] = 0; // Clear adjacent cells too
            }
        }
    }

    while (placed < mines) {
        uint32_t r = rand32() % rows;
        uint32_t c = rand32() % cols;

        // Skip if this is the safe cell or adjacent to it
        if (abs((int)r - safe_r) <= 1 && abs((int)c - safe_c) <= 1) {
            continue;
        }

        if (!mine_grid[r][c]) {
            mine_grid[r][c] = 1;
            placed++;
        }
    }
}

void compute_adj(int rows, int cols) {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (mine_grid[r][c]) { adj[r][c] = 0xFF; continue; }
            int cnt = 0;
            for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                int rr = r + dr, cc = c + dc;
                if (rr >= 0 && rr < rows && cc >= 0 && cc < cols) {
                    if (mine_grid[rr][cc]) cnt++;
                }
            }
            adj[r][c] = (uint8_t)cnt;
        }
    }
}

void flood_reveal(int sr, int sc) {
    if (sr < 0 || sr >= g_rows || sc < 0 || sc >= g_cols) return;
    if (state_grid[sr][sc] == REVEALED || state_grid[sr][sc] == FLAGGED) return;
    if (mine_grid[sr][sc]) return;

    // Simple stack for malloc etc.
    int *stack_r = (int*)0; 
    static int st_r[GRID_MAX_ROWS * GRID_MAX_COLS];
    static int st_c[GRID_MAX_ROWS * GRID_MAX_COLS];
    stack_r = st_r;
    int *stack_c = st_c;
    int top = 0;
    stack_r[top] = sr;
    stack_c[top] = sc;
    top++;
    while (top > 0) {
        top--;
        int r = stack_r[top];
        int c = stack_c[top];
        if (r < 0 || r >= g_rows || c < 0 || c >= g_cols) continue;
        if (state_grid[r][c] == REVEALED || state_grid[r][c] == FLAGGED) continue;
        state_grid[r][c] = REVEALED;
        revealed_count++;
        if (adj[r][c] == 0) {
            for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                int rr = r + dr, cc = c + dc;
                if (rr >= 0 && rr < g_rows && cc >= 0 && cc < g_cols) {
                    if (state_grid[rr][cc] == HIDDEN && !mine_grid[rr][cc]) {
                        stack_r[top] = rr;
                        stack_c[top] = cc;
                        top++;
                    }
                }
            }
        }
    }
}

void reveal_cell(int r, int c) {
    if (r < 0 || r >= g_rows || c < 0 || c >= g_cols) return;
    if (state_grid[r][c] == REVEALED) return;
    if (state_grid[r][c] == FLAGGED) return;

    //Place mines at first reveal to ensure first cell is not a mine
    if (first_move) {
        place_mines(g_rows, g_cols, g_mines, r, c);
        compute_adj(g_rows, g_cols);
        first_move = 0;
    }

    if (mine_grid[r][c]) {
        // Click on mine
        game_over = 1;
        // Eeveal all mines
        for (int rr = 0; rr < g_rows; ++rr)
            for (int cc = 0; cc < g_cols; ++cc)
                if (mine_grid[rr][cc]) state_grid[rr][cc] = REVEALED;
        return;
    }

    if (adj[r][c] == 0) {
        flood_reveal(r, c);
    } else {
        state_grid[r][c] = REVEALED;
        revealed_count++;
    }
    int total = g_rows * g_cols;
    if (revealed_count >= total - g_mines) {
        game_over = 2;
    }
}

void toggle_flag(int r, int c) {
    if (r < 0 || r >= g_rows || c < 0 || c >= g_cols) return;
    if (state_grid[r][c] == REVEALED) return;
    if (state_grid[r][c] == HIDDEN) state_grid[r][c] = FLAGGED;
    else if (state_grid[r][c] == FLAGGED) state_grid[r][c] = HIDDEN;
}

void start_new_game(SudokuDifficulty d) {
    first_move = 1;
    clear_board_state();

    int diff_index = (int)d;
    LevelSpec spec = LEVELS[diff_index];
    g_cols = spec.cols;
    g_rows = spec.rows;
    g_mines = spec.mines;
    if (g_cols > GRID_MAX_COLS) g_cols = GRID_MAX_COLS;
    if (g_rows > GRID_MAX_ROWS) g_rows = GRID_MAX_ROWS;

    // Center cursor
    cursor_r = g_rows / 2;
    cursor_c = g_cols / 2;
    revealed_count = 0;
    game_over = 0;
}

// Read switches
inline uint32_t read_switches(void) {
    return *SW_REG;
}

// Read keys
inline uint32_t read_keys(void) {
    return *KEY_REG;
}

// Poll until key released to avoid multi-fire from a single press
void wait_key_release_all(void) {
    // wait until all keys are 0 (not pressed)
    while (read_keys() != 0) {
        busy_wait(1000);
    }
}

int minesweeper(void) {
    busy_wait(100000);

    // Get difficulty from main menu selection
    SudokuDifficulty diff = get_selected_difficulty_from_switches();
    start_new_game(diff);
    render_board();

    uint32_t prev_keys = 0;
    int needs_redraw = 1;
    int game_over_counter = 0;
    const int GAME_OVER_DELAY = 150;

    while (1) {
        if (needs_redraw) {
            render_board();
            needs_redraw = 0;
        }

        // Game over handling
        if (game_over != 0) {
            game_over_counter++;
            
            if (game_over_counter <= GAME_OVER_DELAY) {
                if (game_over == 1) {
                    draw_text(SCREEN_W/2 - 40, SCREEN_H/2, "GAME OVER", red);
                } else {
                    draw_text(SCREEN_W/2 - 40, SCREEN_H/2, "YOU WIN!", green);
                }
            } else {
                // Return to main menu
                menu_state = MENU_STATE_MAIN;
                return 0;
            }
            
            busy_wait(50000);
            continue;
        }

        // Normal game input processing
        uint32_t sw = read_switches();
        uint32_t keys = read_keys();

        uint32_t key_pressed = keys & (1u << KEY_enter);
        uint32_t prev_key_pressed = prev_keys & (1u << KEY_enter);

        if (key_pressed && !prev_key_pressed) {
            needs_redraw = 1;
            
            if (sw & SW_MASK(SW_up)) {
                if (cursor_r > 0) cursor_r--;
            } else if (sw & SW_MASK(SW_down)) {
                if (cursor_r < g_rows - 1) cursor_r++;
            } else if (sw & SW_MASK(SW_left)) {
                if (cursor_c > 0) cursor_c--;
            } else if (sw & SW_MASK(SW_right)) {
                if (cursor_c < g_cols - 1) cursor_c++;
            } else if (sw & SW_MASK(SW_ACTION_1)) {
                toggle_flag(cursor_r, cursor_c);
            } else if (sw & SW_MASK(SW_ACTION_2)) {
                reveal_cell(cursor_r, cursor_c);
            }
        }

        prev_keys = keys;
        busy_wait(30000);
    }

    return 0;
}