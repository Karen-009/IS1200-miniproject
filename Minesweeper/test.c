/*
  minesweeper_fpga.c

  Bare-metal Minesweeper for DE10-Lite (320x240 8bpp framebuffer).

  Assumptions & notes:
  - Framebuffer: 320 x 240, 1 byte per pixel (8-bit palette index)
    Base address: VGA_Buffer (defined in the header below).
  - Switches and keys are read by reading 32-bit words from SWITCH_BASE, KEY1_base.
    The numeric values in the provided header are treated as bit indices (0..31).
    Example: SW_up == 4 -> check (switch_val & (1u<<4))
  - KEY_enter is active HIGH (1 when pressed).
  - Movement requires direction switch ON + key press (as requested).
  - Uses small 5x7 font for digits 1..8. Simple colored rendering of cells:
      - Hidden cell: dark_gray
      - Flagged: red (flag symbol)
      - Revealed empty: light_gray
      - Revealed number: font drawn with color depending on the number
      - Mine symbol: black star on red background when revealed by loss
  - Difficulty selection at start via SW_l1/SW_l2/SW_l3.
  - Build/port notes: remove/replace any stdlib calls if your runtime doesn't provide them.
*/

#include <stdint.h>
#include <stddef.h>
# include "header.h"

/* ----- Board-specific header (user provided) ----- */
/* ----- end of header ----- */

/* Memory mapped IO helpers */
#define SW_REG  ((volatile uint32_t*) SWITCH_BASE)
#define KEY_REG ((volatile uint32_t*) KEY1_base)
#define VGA_FB  ((volatile uint8_t*) VGA_Buffer)
#define SCREEN_W 320
#define SCREEN_H 240

/* Utility: treat defined switch numbers as bit indices */
#define SW_MASK(x) (1u << (x))

/* Game configuration */
typedef enum { EASY=0, MEDIUM=1, HARD=2 } Difficulty;

/* Choose sizes that fit into 320x240 with cell_size = 12 */
#define CELL_SIZE 12
#define GRID_MAX_COLS (SCREEN_W / CELL_SIZE)   /* = 26 */
#define GRID_MAX_ROWS (SCREEN_H / CELL_SIZE)  /* = 20 */

/* We'll pick these grid sizes for difficulties (col, row, mines) */
typedef struct {
    int cols, rows;
    int mines;
} LevelSpec;

static const LevelSpec LEVELS[3] = {
    /* EASY  */ {9, 9, 10},
    /* MEDIUM*/ {16, 12, 30},
    /* HARD  */ {24, 16, 70}
};

/* Board state */
#define MAX_CELLS (GRID_MAX_ROWS * GRID_MAX_COLS)
static uint8_t mine_grid[GRID_MAX_ROWS][GRID_MAX_COLS]; // 1 = mine, 0 = empty
static uint8_t adj[GRID_MAX_ROWS][GRID_MAX_COLS];       // adjacency counts (0..8)
typedef enum { HIDDEN=0, REVEALED=1, FLAGGED=2 } CellState;
static uint8_t state_grid[GRID_MAX_ROWS][GRID_MAX_COLS];

static int g_rows = 0, g_cols = 0, g_mines = 0;
static int revealed_count = 0;
static int game_over = 0; // 0 running, 1 lost, 2 won

/* Cursor */
static int cursor_r = 0, cursor_c = 0;

/* Simple LFSR PRNG for embedded */
static uint32_t lfsr = 0xACE1u;
static uint32_t rand32(void) {
    /* 16-bit LFSR tapped at 16,14,13,11 (example) expanded to 32-bit mixing */
    uint32_t bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1u;
    lfsr = (lfsr >> 1) | (bit << 15);
    /* mix to 32-bit */
    uint32_t r = lfsr;
    r ^= (r << 5);
    r ^= (r >> 11);
    r ^= (r << 7);
    return r;
}

/* small delay loop (not precise) */
static void busy_wait(volatile int n) {
    while (n-- > 0) {
        asm volatile("nop");
    }
}

/* draw pixel */
static inline void put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
    VGA_FB[y * SCREEN_W + x] = color;
}

/* draw filled rectangle */
static void fill_rect(int x0, int y0, int w, int h, uint8_t color) {
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

/* 5x7 font for digits 0-9 stored as 5 bytes (LSB at top) */
static const uint8_t font5x7_digits[10][5] = {
    {0x7E,0x81,0x81,0x81,0x7E}, // 0 - actually 8x7 but we'll use subset
    {0x00,0x82,0xFF,0x80,0x00}, // 1 (approx)
    {0xE2,0x91,0x91,0x91,0x8E}, // 2
    {0x42,0x81,0x89,0x89,0x76}, // 3
    {0x18,0x14,0x12,0xFF,0x10}, // 4
    {0x4F,0x89,0x89,0x89,0x71}, // 5
    {0x7E,0x89,0x89,0x89,0x72}, // 6
    {0x01,0x01,0xF1,0x09,0x07}, // 7
    {0x76,0x89,0x89,0x89,0x76}, // 8
    {0x46,0x89,0x89,0x89,0x7E}  // 9
};

/* draw a small glyph (digit) centered in a cell */
static void draw_digit_in_cell(int grid_r, int grid_c, int digit, uint8_t color) {
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

/* draw cell border */
static void draw_cell_border(int r, int c, uint8_t border_color) {
    int x0 = c * CELL_SIZE;
    int y0 = r * CELL_SIZE;
    /* top and bottom */
    for (int x = x0; x < x0 + CELL_SIZE; ++x) {
        put_pixel(x, y0, border_color);
        put_pixel(x, y0 + CELL_SIZE - 1, border_color);
    }
    for (int y = y0; y < y0 + CELL_SIZE; ++y) {
        put_pixel(x0, y, border_color);
        put_pixel(x0 + CELL_SIZE - 1, y, border_color);
    }
}

/* render whole board */
static void render_board(void) {
    /* background */
    fill_rect(0, 0, SCREEN_W, SCREEN_H, dark_blue);

    /* draw cells */
    for (int r = 0; r < g_rows; ++r) {
        for (int c = 0; c < g_cols; ++c) {
            int x0 = c * CELL_SIZE;
            int y0 = r * CELL_SIZE;
            /* cell interior */
            if (state_grid[r][c] == HIDDEN) {
                fill_rect(x0 + 1, y0 + 1, CELL_SIZE - 2, CELL_SIZE - 2, dark_gray);
            } else if (state_grid[r][c] == FLAGGED) {
                fill_rect(x0 + 1, y0 + 1, CELL_SIZE - 2, CELL_SIZE - 2, red);
                /* small flag: 3x5 block */
                int fx = x0 + (CELL_SIZE - 3) / 2;
                int fy = y0 + (CELL_SIZE - 5) / 2;
                fill_rect(fx, fy, 1, 5, white);
                fill_rect(fx+1, fy, 2, 3, yellow);
            } else if (state_grid[r][c] == REVEALED) {
                if (mine_grid[r][c]) {
                    fill_rect(x0 + 1, y0 + 1, CELL_SIZE - 2, CELL_SIZE - 2, light_red);
                    /* mine dot in center */
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
                        /* choose color by number */
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
            /* border */
            draw_cell_border(r, c, black);
        }
    }

    /* draw cursor (inverted border) */
    int cx0 = cursor_c * CELL_SIZE;
    int cy0 = cursor_r * CELL_SIZE;
    /* draw a highlighted border */
    for (int x = cx0; x < cx0 + CELL_SIZE; ++x) {
        put_pixel(x, cy0, light_yellow);
        put_pixel(x, cy0 + CELL_SIZE - 1, light_yellow);
    }
    for (int y = cy0; y < cy0 + CELL_SIZE; ++y) {
        put_pixel(cx0, y, light_yellow);
        put_pixel(cx0 + CELL_SIZE - 1, y, light_yellow);
    }
}

/* initialize board arrays */
static void clear_board_state(void) {
    for (int r = 0; r < GRID_MAX_ROWS; ++r) {
        for (int c = 0; c < GRID_MAX_COLS; ++c) {
            mine_grid[r][c] = 0;
            adj[r][c] = 0;
            state_grid[r][c] = HIDDEN;
        }
    }
}

/* place mines randomly */
static void place_mines(int rows, int cols, int mines) {
    int placed = 0;
    while (placed < mines) {
        uint32_t r = rand32() % rows;
        uint32_t c = rand32() % cols;
        if (!mine_grid[r][c]) {
            mine_grid[r][c] = 1;
            placed++;
        }
    }
}

/* compute adjacency */
static void compute_adj(int rows, int cols) {
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

/* flood fill reveal using stack (iterative) */
static void flood_reveal(int sr, int sc) {
    if (sr < 0 || sr >= g_rows || sc < 0 || sc >= g_cols) return;
    if (state_grid[sr][sc] == REVEALED || state_grid[sr][sc] == FLAGGED) return;
    if (mine_grid[sr][sc]) return;

    // simple stack
    int stack_size = g_rows * g_cols;
    int *stack_r = (int*)0; // We don't have malloc; use a static local buffer
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

/* reveal a single cell */
static void reveal_cell(int r, int c) {
    if (r < 0 || r >= g_rows || c < 0 || c >= g_cols) return;
    if (state_grid[r][c] == REVEALED) return;
    if (state_grid[r][c] == FLAGGED) return;
    if (mine_grid[r][c]) {
        // stepped on mine
        game_over = 1;
        // reveal all mines
        for (int rr = 0; rr < g_rows; ++rr)
            for (int cc = 0; cc < g_cols; ++cc)
                if (mine_grid[rr][cc]) state_grid[rr][cc] = REVEALED;
        return;
    }
    if (adj[r][c] == 0) flood_reveal(r, c);
    else {
        state_grid[r][c] = REVEALED;
        revealed_count++;
    }
    int total = g_rows * g_cols;
    if (revealed_count >= total - g_mines) {
        game_over = 2;
        // optionally reveal mines as flagged
    }
}

/* toggle flag */
static void toggle_flag(int r, int c) {
    if (r < 0 || r >= g_rows || c < 0 || c >= g_cols) return;
    if (state_grid[r][c] == REVEALED) return;
    if (state_grid[r][c] == HIDDEN) state_grid[r][c] = FLAGGED;
    else if (state_grid[r][c] == FLAGGED) state_grid[r][c] = HIDDEN;
}

/* initialize new game at difficulty */
static void start_new_game(Difficulty d) {
    clear_board_state();
    LevelSpec spec = LEVELS[d];
    g_cols = spec.cols;
    g_rows = spec.rows;
    g_mines = spec.mines;
    if (g_cols > GRID_MAX_COLS) g_cols = GRID_MAX_COLS;
    if (g_rows > GRID_MAX_ROWS) g_rows = GRID_MAX_ROWS;
    // center cursor
    cursor_r = g_rows / 2;
    cursor_c = g_cols / 2;
    revealed_count = 0;
    game_over = 0;
    // place mines and compute adj
    place_mines(g_rows, g_cols, g_mines);
    compute_adj(g_rows, g_cols);
}

/* read switches */
static inline uint32_t read_switches(void) {
    return *SW_REG;
}

/* read keys */
static inline uint32_t read_keys(void) {
    return *KEY_REG;
}

/* helper: poll until key released to avoid multi-fire from a single press */
static void wait_key_release_all(void) {
    // wait until all keys are 0 (not pressed)
    while (read_keys() != 0) {
        busy_wait(1000);
    }
}

/* choose difficulty by switches at start */
static Difficulty choose_difficulty_from_switches(void) {
    uint32_t sw = read_switches();
    if (sw & SW_MASK(SW_l3)) return HARD;
    if (sw & SW_MASK(SW_l2)) return MEDIUM;
    if (sw & SW_MASK(SW_l1)) return EASY;
    return EASY;
}

/* main game loop */
int main(void) {
    // small startup delay
    busy_wait(100000);

    // pick difficulty at start
    Difficulty diff = choose_difficulty_from_switches();
    start_new_game(diff);
    // initial render
    render_board();

    // previous key snapshot for edge detection (we want press events)
    uint32_t prev_keys = 0;

    while (1) {
        // render every loop (simple)
        render_board();

        // read inputs
        uint32_t sw = read_switches();
        uint32_t keys = read_keys();

        /* movement: require the corresponding switch ON and key pressed (KEY_enter bit) */
        uint32_t key_pressed = keys & (1u << KEY_enter);
        uint32_t prev_key_pressed = prev_keys & (1u << KEY_enter);

        if (key_pressed && !prev_key_pressed) {
            // on rising edge -- process moves/actions according to which switches are ON
            if (sw & SW_MASK(SW_up)) {
                if (cursor_r > 0) cursor_r--;
            } else if (sw & SW_MASK(SW_down)) {
                if (cursor_r < g_rows - 1) cursor_r++;
            } else if (sw & SW_MASK(SW_left)) {
                if (cursor_c > 0) cursor_c--;
            } else if (sw & SW_MASK(SW_right)) {
                if (cursor_c < g_cols - 1) cursor_c++;
            } else if (sw & SW_MASK(SW_ACTION_1)) {
                // toggle flag
                toggle_flag(cursor_r, cursor_c);
            } else if (sw & SW_MASK(SW_ACTION_2)) {
                // reveal
                reveal_cell(cursor_r, cursor_c);
            }
            // if user presses key with no switches, do nothing
        }

        // check for game over -> display final board and halt or restart if they press key
        if (game_over != 0) {
            render_board();
            // small text-free indication: blink whole screen a few times
            for (int i = 0; i < 6; ++i) {
                busy_wait(200000);
                if (game_over == 1) fill_rect(0, 0, SCREEN_W, SCREEN_H, light_red);
                else fill_rect(0, 0, SCREEN_W, SCREEN_H, light_green);
                busy_wait(200000);
                render_board();
            }
            // wait for user to press KEY_enter to restart, using same difficulty switches
            while (!(read_keys() & (1u << KEY_enter))) { busy_wait(1000); }
            // wait release
            wait_key_release_all();
            diff = choose_difficulty_from_switches();
            start_new_game(diff);
            prev_keys = 0;
            continue;
        }

        prev_keys = keys;
        // small loop delay to throttle
        busy_wait(30000);
    }

    return 0;
}


void handle_interrupt(void) {
}

