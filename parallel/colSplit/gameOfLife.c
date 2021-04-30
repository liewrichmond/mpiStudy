#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "../utils.h"

#define NROWS 6
#define NCOLS 6
#define ITERATIONS 3

int **init2DArray(int *buffer,int nRows, int nCols) {
    int **twoDimArray = calloc(nRows, sizeof(*twoDimArray));
    
    for (int i = 0; i < nRows; i++) {
        twoDimArray[i] = buffer + (i*(nCols+2)) + 1;
    }

    return twoDimArray;
}

int *initLocalBuffer(int nRows, int nCols, int nProcesses) {
    int nColsLocal = NCOLS / nProcesses;
    int *buffer = calloc((nRows * (nColsLocal + 2)), sizeof(*buffer));
    return buffer;
}

MPI_Datatype commitGhostCols(int nRows, int nColsLocal) {
    MPI_Datatype ghostCols;
    MPI_Type_vector(nRows, 1, nColsLocal + 2, MPI_INT, &ghostCols);
    MPI_Type_commit(&ghostCols);
    return ghostCols;
}

MPI_Datatype commitLocalStates(int nRows, int nColsGlobal, int nColsLocal) {
    MPI_Datatype localState;
    MPI_Type_vector(nRows, nColsLocal, nColsGlobal, MPI_INT, &localState);
    MPI_Type_commit(&localState);
    return localState;
}

MPI_Datatype commitLocalStateRecv(int nRows, int nColsLocal) {
    MPI_Datatype localStateRecv;
    MPI_Type_vector(nRows, nColsLocal, nColsLocal + 2, MPI_INT, &localStateRecv);
    MPI_Type_commit(&localStateRecv);
    return localStateRecv;
}

int initializeBoard(int *localState, int nRowsGlobal, int nColsGlobal, int currProcess, int nProcesses) {
    int *globalState = calloc(nRowsGlobal * nColsGlobal, sizeof(*globalState));
    int nColsLocal = nColsGlobal / nProcesses;

    MPI_Datatype localStates = commitLocalStates(nRowsGlobal, nColsGlobal, nColsLocal);
    MPI_Datatype localStateRecv = commitLocalStateRecv(nRowsGlobal, nColsLocal);

    if(currProcess == 0) {
        initBoard(globalState, nRowsGlobal, nColsGlobal);
        
        printState(globalState, nRowsGlobal, nColsGlobal);
        
        MPI_Request r;
        for(int i = 0; i < nProcesses; i++) {
            MPI_Isend(globalState + (i*nColsLocal), 1, localStates, i, 0, MPI_COMM_WORLD, &r);
        }
        
    }

    //MPI_Scatter(globalState, 1, localStates, localState, 1, localStates, 0,MPI_COMM_WORLD);
    MPI_Status status;
    MPI_Recv(localState, 1, localStateRecv, 0,0,MPI_COMM_WORLD, &status);
    
    return 0;
}

struct coord **initNeighborArr() {
    int nNeighbors = 8;
    struct coord **neighbors = calloc(nNeighbors, sizeof(*neighbors));
    for(int i = 0; i < nNeighbors; i++) {
        neighbors[i] = calloc(1, sizeof(**neighbors));
    }

    return neighbors;
}

int isAlive(struct coord *neighbor, int nRows, int nCols, int **currState) {
}

int main() {
    int currProcess, nProcesses;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &currProcess);
    
    int nColsLocal = NCOLS/nProcesses;
    int *currStateBuffer = initLocalBuffer(NROWS, NCOLS, nProcesses);
    int *nextStateBuffer = initLocalBuffer(NROWS, NCOLS, nProcesses);

    int **curr = init2DArray(currStateBuffer, NROWS, nColsLocal);
    int **next = init2DArray(nextStateBuffer, NROWS, nColsLocal);
    
    initializeBoard(*curr, NROWS, NCOLS, currProcess, nProcesses);
    
    struct coord **neighbors = initNeighborArray();
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
        for(int j = 0; j < nColsLocal; j++) {
            printf("%d,",curr[i][j]);
        }
        printf("\n");
    }

    free(currStateBuffer);
    free(curr);
    free(nextStateBuffer);
    free(next);
}

