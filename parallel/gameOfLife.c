#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include "time.h"

#define NROWS 1000 
#define NCOLS 1000
#define NITERATIONS 1000
#define ROOTPROCESS 0

// Initializes a a board with either 0 or 1 randomly
void initBoard(int* buffer, size_t nRows, size_t nCols) {
    srand(time(NULL));
    for(int i = 0 ; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            *buffer++ = rand()%2;
        }
    }
}

// Helper Function to prtint the state of a board
void printState(int* buffer, size_t nRows, size_t nCols) {
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            printf("%d,", buffer[(i*nCols) + j]);
        }
        printf("\n");
    }
}

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

// Mutates nieghbors to hold the neighboring indexes of a given 1D index 
void getNeighbors(int cellIndex, int nRows, int nCols, int* neighbors) {
    int rowIndex = cellIndex / nCols;
    int colIndex = cellIndex % nCols;
    
    if (rowIndex > nRows || colIndex > nCols) {
        printf("Invalid Index");
        return;
    }

    int startingRow = rowIndex - 1;
    int startingCol = colIndex - 1;

    if (startingRow < -1 || startingRow > nRows) {
        exit(1);
    }

    if (startingCol < -1 || startingCol > nCols) {
        exit(1);
    }

    for (int i = startingRow; i < startingRow + 3; i++) {
        for (int j = startingCol; j < startingCol + 3; j++) {
            int neighborIndex = getCellIndex(i, j, nCols);
            if (cellIndex != neighborIndex) {
                *neighbors++ = neighborIndex;
            }
        }
    }
}

// Shares the current state information with relevant processes. A given process has to share information with its
// "top" process: rank-1 and "bottom" process: rank+1.
// Implements wrapping such that negative ranks refer to the max rank, and overflowing ranks are "recounted" from rank 0 
int broadcastState(int *localTopRow, int *localBottomRow, int currProcess, int nProcesses, MPI_Request *requests)
{
    int parallelProcessTop = currProcess ? currProcess-1 : nProcesses-1;
    int parallelProcessBottom = currProcess == nProcesses-1 ? 0 : currProcess + 1;
    int receivingTopRow = 0;
    int receivingBottomRow = 1;
    
    MPI_Request *requestPtr = requests;
    MPI_Isend(localTopRow, NCOLS, MPI_INT, parallelProcessTop, receivingBottomRow, MPI_COMM_WORLD, requestPtr++);
    MPI_Isend(localBottomRow, NCOLS, MPI_INT, parallelProcessBottom, receivingTopRow, MPI_COMM_WORLD, requestPtr++);
    
    int nRequests = requestPtr - requests;
    return nRequests;
}

// Receives relevant State information from parallel processes. Mutates parallelTopRow and parallelBottomRow to hold
// information about the top and bottom rows of "ghost cells".
int receiveStates(int *parallelTopRow, int *parallelBottomRow, int currProcess, int nProcesses) 
{
    MPI_Status status;
    
    int parallelProcessTop = currProcess ? currProcess-1 : nProcesses-1;
    int parallelProcessBottom = currProcess == nProcesses-1 ? 0 : currProcess + 1;
    int receivingTopRow = 0;
    int receivingBottomRow = 1;

    MPI_Recv(parallelTopRow, NCOLS, MPI_INT, parallelProcessTop, receivingTopRow, MPI_COMM_WORLD, &status);
    MPI_Recv(parallelBottomRow, NCOLS, MPI_INT, parallelProcessBottom, receivingBottomRow, MPI_COMM_WORLD, &status);   

    return 0;
}

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

// Initializes the state of the 'board' in the root process, and distributes this to all other processes.
void initializeState(int* localState, int localBufferSize, int currProcess, int nProcesses)
{
    MPI_Status status;

    if(currProcess == ROOTPROCESS) {
        //printf("From root Process:\n");
        int buffer[NROWS*NCOLS];
        int *buffPtr = buffer;
        
        initBoard(buffer, NROWS, NCOLS);

        for(int i = 0; i < localBufferSize; i++) {
            localState[i] = *buffPtr++;
        }

        for (int process = 1; process < nProcesses; process++) {
            // Queue Sends here and receive in separate loop?
            MPI_Send(buffPtr, localBufferSize, MPI_INT, process, 0, MPI_COMM_WORLD);
            buffPtr+=localBufferSize;
        }
        //printState(buffer, NROWS, NCOLS);

    }else {
        MPI_Recv(localState, localBufferSize, MPI_INT, ROOTPROCESS, 0, MPI_COMM_WORLD, &status);
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

// Returns the next state of a cell, based on the number of alive neighbors
int getNextState(int currState, int nAlive) {
    int nextState;
    if (nAlive == 3) {
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

// Swaps the pointers to two arrays. This is done to avoid having to do a deep copy.
void swap(int** currState, int** nextState) {
    int* temp = *currState;
    *currState = *nextState;
    *nextState = temp;
}

int main() {
    int currProcess;
    int nProcesses;
    MPI_Request requests[5] = {NULL};

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &currProcess);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    int nRowsLocal = NROWS/nProcesses;
    int localBufferSize = nRowsLocal * NCOLS;

    int currStateLocal[localBufferSize];
    int newStateLocal[localBufferSize];

    int *currStatePtr = currStateLocal;
    int *newStatePtr = newStateLocal;
    
    initializeState(currStatePtr, localBufferSize, currProcess, nProcesses);
    
    int neighbors[8];
    int parallelStateTop[NCOLS];
    int parallelStateBottom[NCOLS];
    int lastRowIndex = localBufferSize - NCOLS;
    int totalAlive;
    for (int iteration = 0; iteration < NITERATIONS; iteration++){ 
        // Since receive states uses a blocking Recv, no need to wait for the requests to complete since it's implied.
        // Still kept the signature to return the number of requests made nonetheless.
        //int nRequests = broadcastState(currStatePtr, currStatePtr+ lastRowIndex, currProcess, nProcesses, requests);
        broadcastState(currStatePtr, currStatePtr+ lastRowIndex, currProcess, nProcesses, requests);
        receiveStates(parallelStateTop, parallelStateBottom, currProcess, nProcesses);

        int nextState;
        int nAlive;
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
        if (iteration == 0) {
            shareNAlive(totalAlive, currProcess, nProcesses);
        }
    }
    
    shareNAlive(totalAlive, currProcess, nProcesses);

    MPI_Finalize();
}
