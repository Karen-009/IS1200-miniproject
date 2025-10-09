# Overview 
This project uses the DE10-Lite FPGA board with switches (SW0-SW9) and KEY1 for navigating through the game. The control mapping for movement and main actions are consistent in both Sudoku and Minesweeper. For movement, the player toggles the switch (on/off), then presses KEY1 to perform the action. Game-specific buttons/switches are: SW8 and SW9. 

# Menu Navigation
- Select Game
SW0: OFF = Minesweeper
     ON  = Sudoku 

- Enter Selected Game
KEY1: Press to start the highlighted game 

# General Controls 
- Exit to Main Menu
press KEY1 

- Select Difficulty (at game start for Sudoku)
SW1: Easy
SW2: Medium
SW3: Hard
Press KEY1 to confirm the desired difficulty level 

- Move Cursor
SW4: Up
SW5: Down
SW6: Right
SW7: Left
To move, turn ON the direction switch, then press KEY1. To move again in any direction, turn the switch OFF, then ON and press KEY1 again. 

# Game-Specific Controls
- Minesweeper
SW8: Flag a cell
SW9: Reveal a cell
Multiple actions can be taken at the same time. For example, moving up a cell and flagging in Minesweeper.

- Sudoku 
SW8: Erase cell 
SW9: Cycle and enter digit
To enter a digit in a cell, turn ON SW9 and press KEY1 repeatedly to cycle through digits 1-9 in the selected cell. The digit is set immediately. When entering the last digit, press KEY1 to get the game state (gameover or you win). 

# Tips
- Always turn OFF a switch before turning ON a new one. 
- Only one movement or action switch should be ON when pressing KEY1, unless combining actions (Minesweeper only). 
- For movement, toggling the same switch and pressing KEY1 again allows repeated moves in the same direction. 