// Main game loop
#include "new.h"

int main() {
    MinesweeperGame game;
    
    // Initialize with easy difficulty
    init_game(&game, DIFFICULTY_EASY);
    
    while(1) {
        handle_input(&game);
        draw_game(&game);
        
        // Simple delay using timer
        volatile int* timer = (volatile int*)TIMER_base;
        int start_time = *timer;
        while(*timer - start_time < 100000); // Adjust for your clock speed
    }
    
    return 0;
}