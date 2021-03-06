#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "../utils/utils.h"
#include "col_utils.h"
#include "messages.h"

#define NROWS 6
#define NCOLS 6
#define ITERATIONS 2

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
    int totalAlive;

    MPI_Datatype ghostCols_t = commitGhostCols(NROWS, nColsLocal);

    for(int iter = 0; iter < ITERATIONS; iter++)
    {
        sendGhostCells(*curr, NCOLS, currProcess, nProcesses, ghostCols_t);
        recvGhostCells(*curr, NCOLS, currProcess, nProcesses, ghostCols_t);
        totalAlive = 0;
        for(int row = 0; row < NROWS; row++)
        {
            for(int col = 0; col < nColsLocal; col++)
            {
                nAlive = 0;
                getNeighbors(row, col, neighbors);
                for(int nIndex = 0; nIndex < 8; nIndex++)
                {
                    neighbor = neighbors[nIndex];
                    if(isAlive(neighbor, NROWS, nColsLocal, curr)) {
                        nAlive++;
                    }
                }
                next[row][col] = getNextState(curr[row][col], nAlive);
                if(next[row][col]) {
                    totalAlive++;
                }
            }
        }

        swap_col(curr, next, NROWS);
    }

    int finalAlive;
    MPI_Reduce(&totalAlive, &finalAlive, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if(currProcess == ROOTPROCESS) {
        printf("Final Alive: %d\n", finalAlive);
    }

    free(currStateBuffer);
    free(curr);
    free(nextStateBuffer);
    free(next);
    freeNeighbors(neighbors);
}
