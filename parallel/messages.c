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

int getParallelRankTop(int currRank, int nProcesses) {
    int paraRankTop = currRank ? currRank - 1 : nProcesses - 1;
    return paraRankTop;
}

int getParallelRankBot(int currRank , int nProcesses) {
    int paraRankBot = currRank == nProcesses - 1 ? 0 : currRank + 1;
    return paraRankBot;
};

int broadcastCurrState(int **currState, int nRows, int nCols, int currRank, int nProcesses, MPI_Request *requests)
{
    int rankIndexTop = getParallelRankTop(currRank, nProcesses);
    int rankIndexBot = getParallelRankBot(currRank, nProcesses);

    MPI_Request *requestPtr = requests;
    MPI_Isend(currState[0], nCols, MPI_INT, rankIndexTop, RECVBOTROW, MPI_COMM_WORLD, requestPtr++);
    MPI_Isend(currState[nRows - 1], nCols, MPI_INT, rankIndexBot, RECVTOPROW, MPI_COMM_WORLD, requestPtr++);
    
    int nRequests = requestPtr - requests;
    return nRequests;
}

int receiveStates(int *parallelTopRow, int *parallelBottomRow, int nCols, int currRank, int nProcesses) 
{
    MPI_Status status;
    
    int rankIndexTop = getParallelRankTop(currRank, nProcesses);
    int rankIndexBot = getParallelRankBot(currRank, nProcesses);

    MPI_Recv(parallelTopRow, nCols, MPI_INT, rankIndexTop, RECVTOPROW, MPI_COMM_WORLD, &status);
    MPI_Recv(parallelBottomRow, nCols, MPI_INT, rankIndexBot, RECVBOTROW, MPI_COMM_WORLD, &status);   

    return 0;
}
