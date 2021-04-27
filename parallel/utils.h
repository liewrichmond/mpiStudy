#include <stdlib.h>
#include <time.h>

struct coord {
    int row;
    int col;
};

void swap(int **curr, int **next);

void initBoard(int *buffer, int nRows, int nCols);
 
void printState(int *buffer, int nRows, int nCols);

int getNextState(int currState, int nAlive);

void getNeighbors(int currRow, int currCol, struct coord **buffer);

void swap(int **curr, int **next);
