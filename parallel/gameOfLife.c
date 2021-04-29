#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "messages.h"
#include "mpi.h"
#include "time.h"

#define NROWS 4 
#define NCOLS 4
#define NITERATIONS 1
#define ROOTPROCESS 0

// Shares the number of cells that are currently alive in a local state, with the root process. 
int shareNAlive(int nAlive, int currProcess, int nProcesses) {
    MPI_Status status;
    if(currProcess == ROOTPROCESS) {
        int totalAlive = nAlive;
        int temp;
        for (int process = 1; process < nProcesses; process++) {
            MPI_Recv(&temp, 1, MPI_INT, process, 0, MPI_COMM_WORLD, &status);
            totalAlive += temp;
        }
        printf("nAlive: %d\n", totalAlive);
    } else {
        MPI_Send(&nAlive, 1, MPI_INT, ROOTPROCESS, 0, MPI_COMM_WORLD);
    }
    return 0;
}

// Shares the current local state with the root process.
void shareNewState(int *localState, int size, int currProcess, int nProcesses) {
    MPI_Status status;

    if(currProcess == ROOTPROCESS) {
        printf("New State: \n");
        int buffer[NROWS*NCOLS];
        int *buffPtr = buffer;
        
        for (int i = 0; i < size; i++) {
            *buffPtr++ = localState[i];
        }

        for (int process = 1; process < nProcesses; process++) {
            MPI_Recv(buffPtr, size, MPI_INT, process, 0, MPI_COMM_WORLD, &status);
            buffPtr+=size;
        }

        printState(buffer, NROWS, NCOLS);
    } else {
        MPI_Send(localState,size, MPI_INT, ROOTPROCESS, 0, MPI_COMM_WORLD);
    }
}

// Returns the current state of a given index. Note that negative indexes and "overflowing" indexes are allowed, to an extent
// Negative indexes refer to the bottom-most row the the preceding process; Overflowing indexes refer to the top-most row of the suceeding process.
int getCurrState(int row, int col, int nRows, int nCols, int **localState, int *parallelStateTop, int *parallelStateBot){
    int state;

    if(col < 0 || col > nCols) {
        col = col < 0 ? nCols - 1 : col - nCols;
    }

    if(row < 0) {
        state = parallelStateTop[col];
    }
    else if (row >= nRows) {
        state = parallelStateBot[col];
    }
    else {
        state = localState[row][col];
    }

    return state;
}


int main(int argc, char *argv[]) {
    int currProcess;
    int nProcesses;
    MPI_Request requests[5];

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &currProcess);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    int nRowsLocal = NROWS/nProcesses;

    int *currStateBuffer = malloc(nRowsLocal* NCOLS * sizeof(*currStateBuffer));
    int *newStateBuffer = malloc(nRowsLocal * NCOLS * sizeof(*newStateBuffer));

    int **curr = malloc(nRowsLocal * sizeof(*curr));
    int **next = malloc(nRowsLocal * sizeof(*next));

    for (int row = 0; row < nRowsLocal; row++) {
        curr[row] = currStateBuffer + (row * NCOLS);
        next[row] = newStateBuffer + (row * NCOLS);
    }
    
    initializeState(*curr, NROWS, NCOLS, currProcess, nProcesses);

    struct coord **neighbors = malloc(8 * sizeof(*neighbors));
    for (int i = 0; i < 8; i++) {
        neighbors[i] = malloc(sizeof(**neighbors));
    }

    int *parallelStateTop = malloc(NCOLS * sizeof(*parallelStateTop));
    int *parallelStateBot = malloc(NCOLS * sizeof(*parallelStateBot));

    int totalAlive;
    int nextState;
    int neighborState;
    struct coord *neighbor;
    int nAlive;

    for (int iteration = 0; iteration < NITERATIONS; iteration++){ 
        broadcastCurrState(*curr, nRowsLocal, NCOLS, currProcess, nProcesses, requests);
        recvCurrState(parallelStateTop, parallelStateBot, NCOLS, currProcess, nProcesses);
        totalAlive = 0;

        for (int row = 0; row < nRowsLocal; row++) {
            for (int col = 0; col < NCOLS; col++) {
                nAlive = 0;
                getNeighbors(row, col, neighbors);
                for(int i = 0; i < 8; i++) {
                    neighbor = neighbors[i];
                    neighborState = getCurrState(neighbor->row, neighbor->col, nRowsLocal, NCOLS, curr, parallelStateTop, parallelStateBot);
                    if(neighborState) {
                        nAlive++;
                    }
                }
                nextState = getNextState(curr[row][col], nAlive);
                next[row][col] = nextState;
            }
        }
        swap(curr, next);
    }
    
    //shareNAlive(totalAlive, currProcess, nProcesses);
    //shareNewState(currStatePtr, localBufferSize, currProcess, nProcesses);

    MPI_Finalize();
}
