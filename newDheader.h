#ifndef DTEKV_BOARD_H
#define DTEKV_BOARD_H

// =====================================================
//                 MEMORY-MAPPED IO
// =====================================================

// Replace these base addresses with your actual FPGA memory map
#define SWITCH_base     0xFF200040
#define KEY1_base       0xFF200050
#define VGA_Buffer      0x08000000
#define VGA_DMA         0xFF203020

// =====================================================
//                 SWITCH & KEY BITMAPS
// =====================================================

// Switch bit positions (adjust if your board is wired differently)
#define SW_up          4   // Move up
#define SW_down        5   // Move down
#define SW_right       6   // Move right
#define SW_left        7   // Move left
#define SW_ACTION_1    8   // Toggle flag
#define SW_ACTION_2    9   // Reveal

// Key bit positions
#define KEY_enter      0   // The ENTER button on your FPGA board

// =====================================================
//                    TYPEDEFS
// =====================================================

typedef enum {
    SW_NONE     = 0,
    SW_UP       = (1 << SW_up),
    SW_DOWN     = (1 << SW_down),
    SW_RIGHT    = (1 << SW_right),
    SW_LEFT     = (1 << SW_left),
    SW_FLAG     = (1 << SW_ACTION_1),
    SW_REVEAL   = (1 << SW_ACTION_2)
} SwitchMap;

#endif // DTEKV_BOARD_H
