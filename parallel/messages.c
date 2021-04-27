#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "messages.h"
#include "utils.h"

void broadcastInitialState(int *globalState, int *localState, int nRows, int nCols, int nProcesses)
{
    memcpy(localState, globalState, nRows*nCols*sizeof(*globalState));
    globalState+= nRows*nCols;
    for(int process = 1; process < nProcesses; process++) {
        MPI_Send(globalState, nRows * nCols, MPI_INT, process, 0, MPI_COMM_WORLD);
        globalState+=nRows*nCols;
    }
}

void receiveInitialState(int *buffer, int nRows, int nCols)
{
    MPI_Status status;
    MPI_Recv(buffer, nRows * nCols, MPI_INT, ROOTPROCESS, 0, MPI_COMM_WORLD, &status);
}

// Initializes the state of the 'board' in the root process, and distributes this to all other processes.
void initializeState(int** localState, int nRows, int nCols, int currProcess, int nProcesses)
{
    if(currProcess == ROOTPROCESS) {
        int *buffer = malloc(nRows* nCols * sizeof(*buffer));
        
        initBoard(buffer, nRows, nCols);
        broadcastInitialState(buffer, *localState, nRows, nCols, nProcesses);
        printState(buffer, nRows, nCols);

        free(buffer);
    }else {
        receiveInitialState(*localState, nRows, nCols);
    }
}
