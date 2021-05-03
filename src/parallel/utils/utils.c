#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

void swap(int **curr, int **next)
{
    int *temp = *next;
    *curr = *next;
    *next = temp;
}

int initBoard(int *buffer, int nRows, int nCols)
{
    srand(time(NULL));
    int nAlive = 0;

    for(int i = 0; i < nRows*nCols; i++) {
        buffer[i] = rand()%2;
        if(buffer[i]) {
            nAlive++;
        }
    }

    return nAlive;
}

void printState(int *buffer, int nRows, int nCols)
{
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            printf("%d, ",buffer[(i*nCols)+j]);
        }
        printf("\n");
    }
    printf("\n");
}

int getNextState(int currState, int nAlive) {
    int nextState;
    if(nAlive == 3) {
        nextState = 1;
    }
    else if (nAlive == 2) {
        nextState = currState;
    }
    else {
        nextState = 0;
    }
    return nextState;
}

void getNeighbors(int currRow, int currCol, struct coord **buffer)
{
    int startingRow = currRow - 1;
    int startingCol = currCol - 1;

    for(int row = startingRow; row < startingRow + 3; row++) {
        for (int col = startingCol; col < startingCol + 3; col++) {
            if(!(col == currCol && row == currRow)) {
                (*buffer)->row = row;
                (*buffer)->col = col;
                ++buffer;
            }
        }
    }
}

struct coord **initNeighbors()
{
    int nNeighbors = 8;
    struct coord **neighbors = calloc(nNeighbors, sizeof(*neighbors));
    for(int i = 0; i < nNeighbors; i++) {
        neighbors[i] = calloc(1, sizeof(**neighbors));
    }

    return neighbors;
}

void freeNeighbors(struct coord **neighbors)
{
    int nNeighbors = 8;
    for(int i = 0; i < nNeighbors; i++ ) {
        free(neighbors[i]);
    }
    free(neighbors);
}
