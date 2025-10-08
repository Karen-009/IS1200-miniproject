# include "minesweeper.h"
# include "dtekv_board.h"

// =====================================================
//                GLOBAL VARIABLES
// =====================================================

// VGA Memory Addresses
volatile char *VGA = (volatile char *) VGA_Buffer;
volatile int  *VGA_ctrl = (volatile int*) VGA_DMA;

// Game state
GameState game;

// Switch and Key Memory Addresses
volatile int *SWITCHES = (volatile int *) SWITCH_base;
volatile int *keys1    = (volatile int *) KEY1_base;

// Previous states for edge detection
static int prev_switches = 0;
static int prev_keys = 0;

// =====================================================
//              ENUM FOR INPUT ACTIONS
// =====================================================

typedef enum {
    INPUT_NONE = 0,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_FLAG,     // Toggle flag
    INPUT_REVEAL,   // Reveal cell
    INPUT_ENTER
} InputAction;

// =====================================================
//                    VGA CONTROL
// =====================================================

void init_vga() {
    *VGA_ctrl = 1;  // Start DMA safely
}

// =====================================================
//              COMPACT DIGIT REPRESENTATIONS
// =====================================================

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

// =====================================================
//               RANDOM NUMBER GENERATOR
// =====================================================

static int seed = 123456789;
void srand_custom(int s) { seed = s; }
int rand() {
    seed = (1103515245 * seed + 12345) & 0x7fffffff;
    return (int)seed;
}

// =====================================================
//                VGA DRAWING ROUTINES
// =====================================================

void draw_pixel(int x, int y, char color) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)
        VGA[y * VGA_WIDTH + x] = color;
}

void clear_screen(char color) {
    for (int i = 0; i < VGA_BUFFER_SIZE; i++)
        VGA[i] = color;
}

void draw_square(int x0, int y0, int width, int height, char color) {
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            draw_pixel(x0 + x, y0 + y, color);
}

void draw_digit(int x, int y, int digit, char color) {
    if (digit < 0 || digit > 9) return;
    for (int row = 0; row < 5; row++) {
        unsigned char row_data = digits_compact[digit][row];
        for (int col = 0; col < 5; col++) {
            if (row_data & (1 << (4 - col)))
                draw_pixel(x + col, y + row, color);
        }
    }
}

// =====================================================
//                GAME INITIALIZATION
// =====================================================

void init_game(int difficulty) {
    game.first_click = 1;
    game.game_over = 0;
    game.game_won = 0;
    game.cursor_x = 0;
    game.cursor_y = 0;

    switch (difficulty) {
        case 0: game.board_size = EASY_SIZE;  game.mine_count = EASY_MINES;  break;
        case 1: game.board_size = MEDIUM_SIZE; game.mine_count = MEDIUM_MINES; break;
        case 2: game.board_size = HARD_SIZE;  game.mine_count = HARD_MINES;  break;
        default: game.board_size = EASY_SIZE; game.mine_count = EASY_MINES;  break;
    }

    for (int i = 0; i < MAX_SIZE; i++)
        for (int j = 0; j < MAX_SIZE; j++) {
            game.grid[i][j] = 0;
            game.revealed[i][j] = 0;
            game.flagged[i][j] = 0;
        }
}

// =====================================================
//                DRAWING GAME BOARD
// =====================================================

void draw_cell(int grid_x, int grid_y) {
    int screen_x = grid_x * CELL_SIZE;
    int screen_y = grid_y * CELL_SIZE;
    char color;

    if (game.revealed[grid_y][grid_x]) {
        if (game.grid[grid_y][grid_x] == -1)
            color = 0x04; // Red for mine
        else
            color = 0x0F; // White for revealed
    } else {
        color = game.flagged[grid_y][grid_x] ? 0x0E : 0x08; // Yellow flag / gray hidden
    }

    draw_square(screen_x, screen_y, CELL_SIZE, CELL_SIZE, color);

    if (game.revealed[grid_y][grid_x] && game.grid[grid_y][grid_x] > 0)
        draw_digit(screen_x + CELL_SIZE / 2 - 2, screen_y + CELL_SIZE / 2 - 2,
                   game.grid[grid_y][grid_x], 0x00);

    // Draw cursor border
    if (grid_x == game.cursor_x && grid_y == game.cursor_y) {
        for (int i = 0; i < CELL_SIZE; i++) {
            draw_pixel(screen_x + i, screen_y, 0x0A);
            draw_pixel(screen_x + i, screen_y + CELL_SIZE - 1, 0x0A);
            draw_pixel(screen_x, screen_y + i, 0x0A);
            draw_pixel(screen_x + CELL_SIZE - 1, screen_y + i, 0x0A);
        }
    }
}

void draw_current_board() {
    for (int y = 0; y < game.board_size; y++)
        for (int x = 0; x < game.board_size; x++)
            draw_cell(x, y);
}

// =====================================================
//                  GAME LOGIC
// =====================================================

void place_mines(int first_x, int first_y) {
    int mines_placed = 0;
    while (mines_placed < game.mine_count) {
        int x = rand() % game.board_size;
        int y = rand() % game.board_size;
        if ((x != first_x || y != first_y) && game.grid[y][x] != -1) {
            game.grid[y][x] = -1;
            mines_placed++;
        }
    }
}

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
                        game.grid[ny][nx] == -1)
                        count++;
                }
                game.grid[y][x] = count;
            }
        }
    }
}

void reveal_all_mines() {
    for (int y = 0; y < game.board_size; y++)
        for (int x = 0; x < game.board_size; x++)
            if (game.grid[y][x] == -1)
                game.revealed[y][x] = 1;
}

// === SAFE REVEAL (ITERATIVE FLOOD FILL) ===
void reveal_cell(int x, int y) {
    if (x < 0 || x >= game.board_size || y < 0 || y >= game.board_size)
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

        if (cx < 0 || cy < 0 || cx >= game.board_size || cy >= game.board_size)
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
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++)
                    if (dx || dy)
                        queue[back][0] = cx + dx, queue[back++][1] = cy + dy;
        }
    }
}

void toggle_flag(int x, int y) {
    if (!game.revealed[y][x])
        game.flagged[y][x] = !game.flagged[y][x];
}

void handle_first_click(int x, int y) {
    place_mines(x, y);
    calculate_adjacent_mines();
    reveal_cell(x, y);
    game.first_click = 0;
}

int check_win_condition() {
    int hidden_safe = 0;
    for (int y = 0; y < game.board_size; y++)
        for (int x = 0; x < game.board_size; x++)
            if (!game.revealed[y][x] && game.grid[y][x] != -1)
                hidden_safe++;
    return (hidden_safe == 0);
}

// =====================================================
//                  INPUT HANDLING
// =====================================================

InputAction get_input_vga(void) {
    int current_switches = *SWITCHES;
    int current_keys     = *keys1;

    InputAction action = INPUT_NONE;

    int key_was_pressed = (prev_keys & (1 << KEY_enter));
    int key_is_pressed  = (current_keys & (1 << KEY_enter));

    int key1_pressed = (!key_was_pressed && key_is_pressed);

    if (key1_pressed) {
        if      (current_switches & (1 << SW_up))         action = INPUT_UP;
        else if (current_switches & (1 << SW_down))       action = INPUT_DOWN;
        else if (current_switches & (1 << SW_right))      action = INPUT_RIGHT;
        else if (current_switches & (1 << SW_left))       action = INPUT_LEFT;
        else if (current_switches & (1 << SW_ACTION_1))   action = INPUT_FLAG;
        else if (current_switches & (1 << SW_ACTION_2))   action = INPUT_REVEAL;
        else                                               action = INPUT_ENTER;
    }

    prev_switches = current_switches;
    prev_keys     = current_keys;

    return action;
}

void handle_input() {
    InputAction action = get_input_vga();

    static int last_difficulty = -1;
    int current_difficulty = get_level();

    if (current_difficulty != last_difficulty) {
        init_game(current_difficulty);
        last_difficulty = current_difficulty;
        draw_current_board();
        return;
    }

    switch (action) {
        case INPUT_UP:    move_cursor(0, -1); break;
        case INPUT_DOWN:  move_cursor(0, 1); break;
        case INPUT_LEFT:  move_cursor(-1, 0); break;
        case INPUT_RIGHT: move_cursor(1, 0); break;
        case INPUT_FLAG:  process_action(SW_ACTION_1); break;
        case INPUT_REVEAL:process_action(SW_ACTION_2); break;
        default: break;
    }
}

void move_cursor(int dx, int dy) {
    int new_x = game.cursor_x + dx;
    int new_y = game.cursor_y + dy;
    if (new_x >= 0 && new_x < game.board_size) game.cursor_x = new_x;
    if (new_y >= 0 && new_y < game.board_size) game.cursor_y = new_y;
}

void process_action(int action) {
    if (game.game_over || game.game_won) return;

    switch (action) {
        case SW_ACTION_1:
            toggle_flag(game.cursor_x, game.cursor_y);
            break;
        case SW_ACTION_2:
            if (game.first_click)
                handle_first_click(game.cursor_x, game.cursor_y);
            else
                reveal_cell(game.cursor_x, game.cursor_y);
            break;
    }

    if (check_win_condition()) game.game_won = 1;
}

// =====================================================
//                   MAIN GAME LOOP
// =====================================================

void game_loop() {
    while (1) {
        handle_input();

        if (!game.game_over && !game.game_won)
            draw_current_board();
        else
            clear_screen(game.game_over ? 0x04 : 0x02); // red = lose, green = win

        for (volatile int i = 0; i < 10000; i++); // small delay
    }
}

// =====================================================
//                      MAIN
// =====================================================

int main() {
    srand_custom(123456789);
    init_vga();
    init_game(0);
    draw_current_board();
    game_loop();
    return 0;
}
