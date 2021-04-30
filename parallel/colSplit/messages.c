#include <mpi.h>
#include "messages.h"
#include "../utils.h"

MPI_Datatype commitGhostCols(int nRows, int nColsLocal)
{
    MPI_Datatype ghostCols;
    MPI_Type_vector(nRows, 1, nColsLocal + 2, MPI_INT, &ghostCols);
    MPI_Type_commit(&ghostCols);
    return ghostCols;
}

MPI_Datatype commitLocalStates(int nRows, int nColsGlobal, int nColsLocal)
{
    MPI_Datatype localState;
    MPI_Type_vector(nRows, nColsLocal, nColsGlobal, MPI_INT, &localState);
    MPI_Type_commit(&localState);
    return localState;
}

MPI_Datatype commitLocalStateRecv(int nRows, int nColsLocal)
{
    MPI_Datatype localStateRecv;
    MPI_Type_vector(nRows, nColsLocal, nColsLocal + 2, MPI_INT, &localStateRecv);
    MPI_Type_commit(&localStateRecv);
    return localStateRecv;
}

void sendInitialState(int *globalState, int nRowsGlobal, int nColsGlobal, int nProcesses)
{ 
    int nColsLocal = nColsGlobal / nProcesses;
    MPI_Datatype localStates = commitLocalStates(nRowsGlobal, nColsGlobal, nColsLocal);
    MPI_Request r;
    for(int i = 0; i < nProcesses; i++) {
        MPI_Isend(globalState + (i*nColsLocal), 1, localStates, i, INITSTATE, MPI_COMM_WORLD, &r);
    }
}

void recvInitialState(int *localState, int nRowsGlobal, int nColsGlobal, int nProcesses)
{
    int nColsLocal = nColsGlobal / nProcesses;
    MPI_Status status;
    MPI_Datatype localStateRecv = commitLocalStateRecv(nRowsGlobal, nColsLocal);
    MPI_Recv(localState, 1, localStateRecv, ROOTPROCESS, INITSTATE, MPI_COMM_WORLD, &status);
}

int initializeBoard(int *localState, int nRowsGlobal, int nColsGlobal, int currProcess, int nProcesses)
{
    if(currProcess == 0) {
        int *globalState = calloc(nRowsGlobal * nColsGlobal, sizeof(*globalState));

        initBoard(globalState, nRowsGlobal, nColsGlobal);
    
        printState(globalState, nRowsGlobal, nColsGlobal);

        sendInitialState(globalState, nRowsGlobal, nColsGlobal, nProcesses);

        recvInitialState(localState, nRowsGlobal, nColsGlobal, nProcesses);
        
        free(globalState);
    }
    else {
        recvInitialState(localState, nRowsGlobal, nColsGlobal, nProcesses);
    }

    return 0;
}

int getLeftProcessRank(int currProcess, int nProcesses)
{
    int leftProcessRank = currProcess == 0 ? nProcesses - 1 : currProcess - 1;
    return leftProcessRank;
}

int getRightProcessRank(int currProcess, int nProcesses)
{
   int rightProcessRank = currProcess == nProcesses - 1 ? 0 : currProcess + 1; 
   return rightProcessRank;
}

void sendGhostCells(int *localState, int nColsGlobal, int currProcess,
        int nProcesses, MPI_Datatype ghostCol)
{
    // Determine who to share with
    int leftRank = getLeftProcessRank(currProcess, nProcesses);
    int rightRank = getRightProcessRank(currProcess, nProcesses);
    int nColsLocal = nColsGlobal / nProcesses;
    int *leftColStart = localState;
    int *rightColStart = &(localState[nColsLocal-1]);

    // Send data
    MPI_Request r;
    MPI_Isend(leftColStart, 1, ghostCol, leftRank, RECVRIGHT, MPI_COMM_WORLD, &r);
    MPI_Isend(rightColStart, 1, ghostCol, rightRank, RECVLEFT, MPI_COMM_WORLD, &r);
}

void recvGhostCells(int *localState, int nColsGlobal, int currProcess, int nProcesses, MPI_Datatype ghostCol)
{
    int leftRank = getLeftProcessRank(currProcess, nProcesses);
    int rightRank = getRightProcessRank(currProcess, nProcesses);
    int nColsLocal = nColsGlobal / nProcesses;
    int *leftGhostCellStart = &(localState[-1]);
    int *rightGhostCellStart = &(localState[nColsLocal]);

    MPI_Status s;
    MPI_Recv(leftGhostCellStart, 1, ghostCol, leftRank, RECVLEFT, MPI_COMM_WORLD, &s);
    MPI_Recv(rightGhostCellStart, 1, ghostCol, rightRank, RECVRIGHT, MPI_COMM_WORLD, &s);
}
