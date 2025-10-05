#include "sudoku_input_vga.h"
#include "dtekv_board.h"

// Switch and Key Memory Addresses
volatile int *SWITCHES = (volatile int *) SWITCH_base;
volatile int *keys1 = (volatile int *) KEY1_base; //address of KEY1

// Debounce state variables, only reacts to new KEY1/SWITCHES presses
static int prev_switches = 0;  
static int prev_keys = 0;   

InputAction get_input_action(void) {
    int current_switches = *SWITCHES;
    int current_keys = *keys1;

    InputAction action = INPUT_NONE;

    int key1_pressed = (current_keys & (1 << KEY_enter)) && !(prev_keys & (1 << KEY_enter));

    if (key1_pressed) {
        if (current_switches & (1 << SW_up))       action = INPUT_UP;
        else if (current_switches & (1 << SW_down))  action = INPUT_DOWN;
        else if (current_switches & (1 << SW_right)) action = INPUT_RIGHT;
        else if (current_switches & (1 << SW_left))  action = INPUT_LEFT;
        else if (current_switches & (1 << SW_ACTION_1))      action = INPUT_ERASE;
        else if (current_switches & (1 << SW_enter_digit))      action = INPUT_INCREMENT;
        else                                     action = INPUT_ENTER; // Default: plain enter
    }

    prev_switches = current_switches;
    prev_keys = current_keys;

    // Ensure the action is valid before returning
    if (action != INPUT_NONE) {
        return action;
    }

    // Default to INPUT_NONE if no valid action is detected
    return INPUT_NONE;
}