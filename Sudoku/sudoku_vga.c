// Contains VGA drawing functions - how to draw the board, numbers, highlights, cursor, timer, end game screens etc. 

#include "dtekv_board.h"   
#include "sudoku_vga.h"
#include "sudoku.h"

// VGA Memory Addresses
volatile char *VGA = (volatile char *) VGA_Buffer;
volatile int *VGA_ctrl = (volatile int*) VGA_DMA;

// Grid and cell dimensions
#define CELL_SIZE 40       // Each cell is 40x40 pixels




