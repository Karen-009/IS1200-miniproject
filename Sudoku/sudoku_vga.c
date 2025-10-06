// Contains VGA drawing functions - how to draw the board, numbers, highlights, cursor, timer, end game screens etc. 

#include "dtekv_board.h" 
#include "sudoku_vga.h"
#include "sudoku.h"

// VGA screen dimensions
#define VGA_WIDTH 320
#define VGA_HEIGHT 240


// Grid layout constants
#define CELL_SIZE 24      // Each cell is 24x24 pixels
#define GRID_ORIGIN_X 28 // Top-left corner of the grid
#define GRID_ORIGIN_Y 20 // Leave space for timer/score at top
#define BOARD_SIZE (CELL_SIZE * SUDOKU_SIZE) // 216 pixels for 9 cells
#define LINE_THICKNESS 2 // Thickness of grid lines


// VGA Memory Addresses
volatile char *VGA = (volatile char *) VGA_Buffer;
volatile int *VGA_ctrl = (volatile int*) VGA_DMA;

// Draw a single pixel at (x, y) with the specified color
void draw_pixel(int x, int y, char color) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        VGA[y * VGA_WIDTH + x] = color;
    }
}

// Draw a filled rectangle starting at (x, y) with given width, height, and color
void draw_rect(int x, int y, int width, int height, char color) {
    for (int dy = 0; dy < height; dy++) {   // dy and dx for better readability, indicates the change in y and x direction
        for (int dx = 0; dx < width; dx++) {
            draw_pixel(x + dx, y + dy, color);
        }
    }
}

// Draw a block, this function ensures proper visual separation of 3x3 sub-grids within the Sudoku board
void draw_block(int x, int y, int width, int height, char color) {
    draw_rect(x, y, width, height, color);
}

// Get background color for a 3x3 box based on its position
char get_box_color(int box_row, int box_col) {
    char box_colors[3][3] = {
        {pastel_pink, light_blue, light_yellow},
        {light_yellow, pastel_pink, light_blue},
        {light_blue, light_yellow, pastel_pink}
    };
    return box_colors[box_row][box_col];
}

// 5x5 bitmap for digits 0–9
static const unsigned char digits_compact[10][5] = {
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

// Helper function to draw a digit centered in a cell
void draw_digit(int grid_x, int grid_y, int number, char color) {
    if(number < 1 || number > 9) return; // Sudoku uses 1–9

    int cell_size = CELL_SIZE;
    int pixel_x = GRID_ORIGIN_X + grid_x * cell_size;
    int pixel_y = GRID_ORIGIN_Y + grid_y * cell_size;

    // Center the 5x5 digit in the cell
    int margin = (cell_size - 5*3) / 2; // scale = 3 px per "bit"
    int start_x = pixel_x + margin;
    int start_y = pixel_y + margin;

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            if ((digits_compact[number][row] >> (4 - col)) & 1) {
                // 3x3 block per "pixel"
                for(int dx=0; dx<3; dx++)
                    for(int dy=0; dy<3; dy++)
                        draw_pixel(start_x + col*3 + dx, start_y + row*3 + dy, color);
            }
        }
    }
}

// Draw a single Sudoku cell at (cell_x, cell_y)
void draw_sudoku_cell(int cell_x, int cell_y, const SudokuGame *game) {
    int cell_size = CELL_SIZE;
    int screen_x = GRID_ORIGIN_X + cell_x * cell_size;
    int screen_y = GRID_ORIGIN_Y + cell_y * cell_size;

    // Determine which 3x3 box this cell is in
    int box_row = cell_y / 3;
    int box_col = cell_x / 3;
    char bg_color = get_box_color(box_row, box_col);

    // Draw cell background
    draw_block(screen_x, screen_y, cell_size, cell_size, bg_color);

    // Draw cell border
    for (int i = 0; i < cell_size; i++) {
        draw_pixel(screen_x + i, screen_y, dark_gray); // Top border
        draw_pixel(screen_x, screen_y + i, dark_gray); // Left border
        draw_pixel(screen_x + i, screen_y + cell_size - 1, dark_gray); // Bottom border
        draw_pixel(screen_x + cell_size - 1, screen_y + i, dark_gray); // Right border
    }

    // Draw number if present
    int value = game->grid.cells[cell_y][cell_x].value; // Note: row = y, col = x
    if (value != 0) {
        char num_color = game->grid.cells[cell_y][cell_x].fixed ? black : blue; // Fixed numbers in black, user entries in blue
        draw_digit(cell_x, cell_y, value, num_color);
    }
}

// Draw bold lines for 3x3 boxes
void draw_bold_grid_lines(void) {
    int cell_size = CELL_SIZE;
    int thickness = LINE_THICKNESS;

    // Vertical bold lines
    for (int i = 0; i <= SUDOKU_SIZE; i += 3) {
        int x = GRID_ORIGIN_X + i * cell_size;
        for (int t = 0; t < thickness; t++) {
            draw_rect(x + t, GRID_ORIGIN_Y, thickness, BOARD_SIZE, black);
        }
    }

    // Horizontal bold lines
    for (int i = 0; i <= SUDOKU_SIZE; i += 3) {
        int y = GRID_ORIGIN_Y + i * cell_size;
        for (int t = 0; t < thickness; t++) {
            draw_rect(GRID_ORIGIN_X, y + t, BOARD_SIZE, thickness, black);
        }
    }
}

// Draw the entire Sudoku board
void draw_sudoku_board(const SudokuGame *game) {
    // Draw background for the board area
    draw_rect(GRID_ORIGIN_X - LINE_THICKNESS, GRID_ORIGIN_Y - LINE_THICKNESS, 
              BOARD_SIZE + 2*LINE_THICKNESS, BOARD_SIZE + 2*LINE_THICKNESS, light_gray);

    // Draw each cell
    for (int row = 0; row < SUDOKU_SIZE; row++) {
        for (int col = 0; col < SUDOKU_SIZE; col++) {
            draw_sudoku_cell(col, row, game);
        }
    }

    // Draw bold grid lines on top
    draw_bold_grid_lines();
}

// Draw cursor around currently selected cell
void draw_cursor(const SudokuGame *game) {
    int cell_size = CELL_SIZE;
    int screen_x = GRID_ORIGIN_X + game->selected_col * cell_size;
    int screen_y = GRID_ORIGIN_Y + game->selected_row * cell_size;

    // Draw yellow border around selected cell
    for (int i = 0; i < cell_size; i++) {
        draw_pixel(screen_x + i, screen_y, yellow);
        draw_pixel(screen_x + i, screen_y + cell_size - 1, yellow);
        draw_pixel(screen_x, screen_y + i, yellow);
        draw_pixel(screen_x + cell_size - 1, screen_y + i, yellow);
    }
}

// Draw "Game Over" or "You Win" screen by overlaying a rectangle in the center of the screen
void draw_game_over(const SudokuGame *game) {
    if (game->state == GAME_LOST) {
        draw_rect(80, 100, 160, 40, red);   // Red rectangle for "Game Over"
    } else if (game->state == GAME_WON) {
        draw_rect(80, 100, 160, 40, green); // Green rectangle for "You Win"
    }
}

// main render function to be called from main loop, draws the entire game state
void sudoku_render_vga(const SudokuGame *game) {
    // Clear screen
    draw_rect(0, 0, VGA_WIDTH, VGA_HEIGHT, white);

    // Draw the Sudoku board
    draw_sudoku_board(game);

    // Draw the cursor if game is running
    if (game->state == GAME_RUNNING) {
        draw_cursor(game);
    }

    // Draw timer and score at the top
    //draw_timer(game);

    // Draw end game screens 
    if (game->state == GAME_WON || game->state == GAME_LOST) {
        draw_game_over(game);
    }

    // Trigger VGA DMA to update the screen
    *VGA_ctrl = 1; // Start DMA transfer
}

