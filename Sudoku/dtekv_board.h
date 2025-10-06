#ifndef DTEKV_BOARD_H
#define DTEKV_BOARD_H
#include <stdlib.h>

// Memory Addresses for VGA
#define VGA_Buffer 0x8000000
#define VGA_DMA 0x4000100
#define SWITCH_base 0x4000010
#define KEY1_base 0x40000d0
#define TIMER_base 0x4000020
#define HEX_base 0x4000050
#define SWITCH_BASE 0x4000010  

// Colors 
#define black     0x00
#define white     0xFF
#define red       0xE0
#define yellow    0xFC
#define green     0x1C
#define blue      0x03
#define cyan      0x1F
#define magenta   0xE3
#define gray      0x92
#define dark_gray 0x49
#define light_blue 0x9F
#define orange    0xFC 
#define light_gray 0xE4 
#define light_green 0x7C
#define light_red 0xF4
#define light_yellow 0xFE
#define purple    0xA3
#define brown     0xB2
#define pink      0xF3
#define light_cyan 0xBF
#define light_magenta 0xF3
#define dark_blue 0x02
#define dark_green 0x0C
#define pastel_pink 0xF5 

// Swich and Key Assignments
#define SW_l1 0     // easy difficulty
#define SW_l2 1    // medium difficulty
#define SW_l3 2    // hard  difficulty
#define SW_up 3    // move cursor up
#define SW_down 4  // move cursor down
#define SW_right 5 // move cursor right
#define SW_left 6  // move cursor left
#define SW_flag 7  // toggle flag mode
#define SW_ACTION_1 8 // (reveal/erase cell)
#define SW_enter_digit 9 // enter digit mode
#define KEY_enter 0 // confirm action
#define KEY_exit 1  // exit game


#endif