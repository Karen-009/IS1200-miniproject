# include "minesweeper.h"
# include "dtekv_board.h"

//VGA Memory Addresses
volatile char *VGA = (volatile char *) VGA_Buffer;
volatile int *VGA_ctrl = (volatile int*) VGA_DMA;

//Switch and Key Memory Addresses
volatile int *SWITCHES = (volatile int *) SWITCH_base;
volatile int *keys1 = (volatile int *) KEY1_base; //address of KEY1

// Compact digit representation using bits (5 bytes per digit)
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

void init_game(int dificulty){
    game.first_click = 1;
    game.game_over = 0;
    game.game_won = 0;
    game.cursor_x = 0;
    game.cursor_y = 0;
    game.last_switches = 0;
    game.last_keys = 0;

    // Set the board size and mine count based on difficulty
    switch (dificulty){
        case 0: //Level 1
            game.board_size = EASY_SIZE;
            game.mine_count = EASY_MINES;
            break;
        case 1: //Level 2
            game.board_size = MEDIUM_SIZE;
            game.mine_count = MEDIUM_MINES;
            break;  
        case 2: //Level 3
            game.board_size = HARD_SIZE;
            game.mine_count = HARD_MINES;
            break;
        default:
            game.board_size = EASY_SIZE;
            game.mine_count = EASY_MINES;
    }

    //Clears all data and resets the game to initial state, runs though all of the cells and sets to initial state
    for(int i = 0; i < MAX_SIZE; i++){
        for(int j = 0; j < MAX_SIZE; j++){
            game.grid[i][j] = 0;
            game.revealed[i][j] = 0;
            game.flagged[i][j] = 0;
        }
    }
}
