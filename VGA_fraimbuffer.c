    printf("   "); // Print column headers
    for (int c = 0; c < grid->cols; c++)
        printf("%2d ", c);
    printf("\n");

    for (int r = 0; r < grid->rows; r++) // Loop through each row
    {
        printf("%2d ", r); // Print row header
        for (int c = 0; c < grid->cols; c++) // Loop through each column
        {
            Cell *cell = getCell(grid, r, c); // Get the cell at (r, c)
            if (cell->revealed) // If the cell is revealed
            {
                if (cell->mine) // If the cell has a mine
                    printf(" * "); // Print mine
                else if (cell->adjacent_mines > 0) // If the cell has adjacent mines
                    printf(" %d ", cell->adjacent_mines); // Print number of adjacent mines
                else
                    printf("   "); // Print empty space for cells with no adjacent mines
            }
            else if (cell->flagged) // If the cell is flagged
            {
                printf(" F "); // Print flag
            }
            else if (reveal_mines && cell->mine) // If reveal_mines is true and the cell has a mine
            {
                printf(" * "); // Print mine
            }
            else
            {
                printf(" . "); // Print unrevealed cell
            }
        }
        printf("\n"); // New line after each row
    }