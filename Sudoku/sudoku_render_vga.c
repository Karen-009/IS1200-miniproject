#include "dtekv_board.h"
#include "sudoku_vga.h"

// VGA Memory Addresses
volatile char *VGA = (volatile char *) VGA_Buffer;
volatile int *VGA_ctrl = (volatile int*) VGA_DMA;

