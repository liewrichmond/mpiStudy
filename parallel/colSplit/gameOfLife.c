#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "../utils.h"
#include "col_utils.h"
#include "messages.h"

#define NROWS 6
#define NCOLS 6
#define ITERATIONS 3

int getCurrState(int currRow, int currCol, int nRows, int nCols, int **currState) {
    //Wrap rows; negative column indexing is fine
    if(currRow < 0 || currRow >= nRows) {
        currRow = currRow < 0 ? nRows - 1 : 0;
    }
    return currState[currRow][currCol];
}

int isAlive(struct coord *neighbor, int nRows, int nCols, int **currState) {
    return getCurrState(neighbor->row, neighbor->col, nRows, nCols, currState);
}

int main() {
    int currProcess, nProcesses;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &currProcess);
    
    int nColsLocal = NCOLS/nProcesses;
    int *currStateBuffer = initLocalBuffer(NROWS, NCOLS, nProcesses);
    int *nextStateBuffer = initLocalBuffer(NROWS, NCOLS, nProcesses);

    int **curr = init2DArray(currStateBuffer, NROWS, NCOLS, nProcesses);
    int **next = init2DArray(nextStateBuffer, NROWS, NCOLS, nProcesses);
    
    initializeBoard(*curr, NROWS, NCOLS, currProcess, nProcesses);
    
    struct coord **neighbors = initNeighbors();
    struct coord *neighbor;
    int nAlive;
    

    for(int row = 0; row < NROWS; row++) {
        for(int col = 0; col < nColsLocal; col++) {
            nAlive = 0;
            getNeighbors(row, col, neighbors);
            for(int nIndex = 0; nIndex < 8; nIndex++) {
                neighbor = neighbors[nIndex];
                if(isAlive(neighbor, NROWS, nColsLocal, curr)) {
                    nAlive++;
                }
                next[row][col] = getNextState(curr[row][col], nAlive);
            }
        }
    }
    swap(curr, next);
    
    printf("From Process %d\n", currProcess);
    for(int i = 0; i < NROWS; i++) {
        for(int j = -1; j < nColsLocal+1; j++) {
            printf("%d,",curr[i][j]);
        }
        printf("\n");
    }

    free(currStateBuffer);
    free(curr);
    free(nextStateBuffer);
    free(next);
    freeNeighbors(neighbors);
}

