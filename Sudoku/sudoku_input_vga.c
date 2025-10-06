#include "sudoku_input_vga.h"
#include "dtekv_board.h"

// Switch and Key Memory Addresses
volatile int *SWITCHES = (volatile int *) SWITCH_base;
volatile int *keys1    = (volatile int *) KEY1_base; // address of KEY1

// Debounce state variables, only reacts to new KEY1/SWITCHES presses
static int prev_switches = 0;  
static int prev_keys     = 0;   

InputAction get_input_vga(void) {
    int current_switches = *SWITCHES;
    int current_keys     = *keys1;

    InputAction action = INPUT_NONE;

    // Key is pressed when the bit is 0, released when 1
    int key_was_pressed = !(prev_keys & (1 << KEY_enter));
    int key_is_pressed  = !(current_keys & (1 << KEY_enter));

    // Detect rising edge of PRESS (released -> pressed)
    int key1_pressed = (!key_was_pressed && key_is_pressed);

    if (key1_pressed) {
        if      (current_switches & (1 << SW_up))         action = INPUT_UP;        // Switch 4
        else if (current_switches & (1 << SW_down))       action = INPUT_DOWN;      // Switch 5
        else if (current_switches & (1 << SW_right))      action = INPUT_RIGHT;     // Switch 6
        else if (current_switches & (1 << SW_left))       action = INPUT_LEFT;      // Switch 7
        else if (current_switches & (1 << SW_ACTION_1))   action = INPUT_ERASE;     // Switch 8
        else if (current_switches & (1 << SW_ACTION_2))   action = INPUT_INCREMENT; // Switch 9
        else                                              action = INPUT_ENTER;    // Default: plain enter
    }

    prev_switches = current_switches;
    prev_keys     = current_keys;

    return action;
}
