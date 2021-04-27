#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "messages.h"
#include "mpi.h"
#include "time.h"

#define NROWS 1000 
#define NCOLS 1000
#define NITERATIONS 1
#define ROOTPROCESS 0


// Returns the 1D Index of a row-column pair. Only Wraps the Columns such that 0,-1 --> 0,nCol-1
// This is done in order to denote a "Ghost Cell" from a parallel Process.
int getCellIndex(int row, int col, int nCols) {
    //Only wrap the column here. Negative/Overflowed row index denotes "ghost cells"
    if (col < 0 || col >= nCols) {
        col = col < 0 ? nCols + col : col - nCols;
    }
    
    int oneDimIndex;
    oneDimIndex = row < 0 ? -col-1 : (row*nCols) + col;
    
    return oneDimIndex;
}


// Shares the current state information with relevant processes. A given process has to share information with its
// "top" process: rank-1 and "bottom" process: rank+1.
// Implements wrapping such that negative ranks refer to the max rank, and overflowing ranks are "recounted" from rank 0 

// Receives relevant State information from parallel processes. Mutates parallelTopRow and parallelBottomRow to hold
// information about the top and bottom rows of "ghost cells".

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
int getCurrState(int index, int localBufferSize, int *localState, int *parallelStateTop, int *parallelStateBottom){
    int state;
    if(index < 0) {
        state = parallelStateTop[-(index+1)];
    }
    else if (index >= localBufferSize) {
        state = parallelStateBottom[index-localBufferSize];
    }
    else {
        state = localState[index];
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
        *curr = currStateBuffer + (row * NCOLS);
    }
    
    initializeState(curr, nRowsLocal, NCOLS, currProcess, nProcesses);
    
    int neighbors[8];
    int *parallelStateTop = malloc(NCOLS * sizeof(*parallelStateTop));
    int *parallelStateBottom = malloc(NCOLS * sizeof(*parallelStateBottom));

    int totalAlive;
    int nextState;
    int nAlive;

    for (int iteration = 0; iteration < NITERATIONS; iteration++){ 
        // Since receive states uses a blocking Recv, no need to wait for the requests to complete since it's implied.
        // Still kept the signature to return the number of requests made nonetheless.
        //int nRequests = broadcastState(currStatePtr, currStatePtr+ lastRowIndex, currProcess, nProcesses, requests);
        broadcastCurrState(curr, NROWS, NCOLS, currProcess, nProcesses, requests);
        receiveStates(parallelStateTop, parallelStateBottom, NCOLS, currProcess, nProcesses);

        totalAlive = 0;

        for (int cell = 0; cell < localBufferSize; cell++) {
            nAlive = 0;
            getNeighbors(cell, nRowsLocal, NCOLS, neighbors);
            for (int neighbor = 0; neighbor < 8; neighbor++) {
                int neighborIndex = neighbors[neighbor];
                if (getCurrState(neighborIndex, localBufferSize, currStatePtr, parallelStateTop, parallelStateBottom)) {
                    nAlive++;
                }
            }
            nextState = getNextState(currStatePtr[cell], nAlive);
            if(nextState) {
                totalAlive += 1;
            }
            newStatePtr[cell] = nextState;
        }

        swap(&currStatePtr, &newStatePtr);
    }
    
    shareNAlive(totalAlive, currProcess, nProcesses);
    //shareNewState(currStatePtr, localBufferSize, currProcess, nProcesses);

    MPI_Finalize();
}
