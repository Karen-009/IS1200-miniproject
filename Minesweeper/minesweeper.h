/* ----- Board-specific header (user provided) ----- */
#ifndef DTEKV_BOARD_H
#define DTEKV_BOARD_H

// Memory Addresses for VGA
#define VGA_Buffer 0x08000000u
#define VGA_DMA 0x4000100u
#define SWITCH_base 0x4000010u
#define KEY1_base 0x40000d0u
#define TIMER_base 0x4000020u
#define HEX_base 0x4000050u
#define SWITCH_BASE 0x4000010u  

// Colors (8-bit palette indices)
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

// Switch and Key Assignments (bit indices)
#define SW_l1 1     // easy difficulty  (bit index)
#define SW_l2 2     // medium difficulty
#define SW_l3 3     // hard  difficulty
#define SW_up 4     // move cursor up
#define SW_down 5   // move cursor down
#define SW_right 6  // move cursor right
#define SW_left 7   // move cursor left
#define SW_ACTION_1 8   // toggle flag / erase cell
#define SW_ACTION_2 9   // reveal cell / enter digit mode
#define KEY_enter 0 // confirm action (bit index)

#endif
/* ----- end of header ----- */