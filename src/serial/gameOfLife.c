#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct board {
    int* cells;
    int nRows;
    int nCols;
}Board;

Board* makeNewBoard(size_t nRows, size_t nCols) {
    Board* board = malloc(sizeof(Board));
    srand(time(NULL));

    if(board) {
        board->nRows = nRows;
        board->nCols = nCols;
        board->cells = malloc(sizeof(int)*nRows*nCols);
        if(board->cells) {
            for (int i = 0; i < (nRows * nCols); i++) {
                (board->cells)[i] = rand() % 2;
            }
        }
    }

    return board;
}

// Returns corresponding 1d index of a given board and row,col pair.
// Implements "wrapping" where a negative row/col refers to a backwards count
int getCellIndex(int row, int col, Board* board) {
    int index = 0;
    if (row < 0 || row >= board->nRows) {
        row = row < 0 ? board->nRows + row : row - board->nRows;
    }

    if (col < 0 || col >= board->nCols) {
        col = col < 0 ? board->nCols + col : col - board->nCols;
    }

    index = (row*board->nCols) + col;
    return index;
}

// Returns the neighboring indexes for a given cell index
void getNeighbors(int* buffer, int index, Board* board)
{
    int rowIndex = index / board->nCols;
    int colIndex = index % board->nCols;

    if (rowIndex > board->nRows || colIndex > board->nCols) {
        printf("Invalid Cell Index!");
        exit(1);
    }
    
    int startingRow = rowIndex - 1;
    int startingCol = colIndex - 1;
    
    int cellIndex = 0;
    for (int i = startingRow ; i < startingRow + 3; i++) {
        for (int j = startingCol ; j < startingCol + 3; j++) {
            cellIndex = getCellIndex(i,j, board);
            if(cellIndex != index) {
                *buffer = cellIndex;
                buffer++;
            }
        }
    }
}

// Returns the next state of a given cell based on it's current state and the number of neighbors that are currently "alive"
int getNextState(int cell, int currState, int nAlive) {
    int newState;
    // Conditionals based on nAlive,
    if (nAlive == 3) {
        newState = 1;
    } else if (nAlive == 2) {
        newState = currState; 
    } else {
        newState = 0;
    }
    return newState; 
}

void swapBoards(Board* curr, Board* new) {
    Board temp;
    temp = *curr;
    *curr = *new;
    *new = temp;
}

void printBoard(Board* board) {
    for(int i = 0; i < board->nRows; i++) {
        for (int j = 0; j < board->nCols; j++) {
            int cell = (i*board->nCols) + j;
            printf("%d ",board->cells[cell]);
        }
        printf("\n");
    }
}

void runSerial()
{
    int nRows = 50;
    int nCols = 50;
    int nIterations = 5000;

    Board* currBoard = makeNewBoard(nRows, nCols);
    Board* newBoard = makeNewBoard(nRows,nCols);
    
    int nNeighbors = 8;
    int neighbors[nNeighbors];
    int nAlive;
    printBoard(currBoard);
    
    for (int iter = 0 ; iter < nIterations; iter ++ ){
        for (int cell = 0 ; cell < nRows * nCols; cell++) {
            nAlive = 0;
            getNeighbors(neighbors, cell, currBoard);
            
            for (int j = 0; j < nNeighbors; j++) {
                int neighbor = neighbors[j];
                int isAlive = currBoard->cells[neighbor];
                if(isAlive) {
                    nAlive++;
                }
            } 

            int nextState = getNextState(cell, currBoard->cells[cell], nAlive);
            newBoard->cells[cell] = nextState;
        }
        
        swapBoards(currBoard, newBoard);
        printf("\n");
        printBoard(currBoard);
    }

    free(currBoard->cells);
    free(newBoard->cells);
    free(currBoard);
    free(newBoard);
}

int main() {
    runSerial();
}
