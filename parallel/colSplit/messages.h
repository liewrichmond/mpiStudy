#include <mpi.h>

#define ROOTPROCESS 0
#define INITSTATE 0
#define RECVLEFT 1
#define RECVRIGHT 2

MPI_Datatype commitGhostCols(int nRows, int nColsLocal);

MPI_Datatype commitLocalStates(int nRows, int nColsGlobal, int nColsLocal);

MPI_Datatype commitLocalStateRecv(int nRows, int nColsLocal);

void sendInitialState(int *globalState, int nRowsGlobal, int nColsGlobal, int nProcesses);

void recvInitialState(int *localState, int nRowsGlobal, int nColsGlobal, int nProcesses);

int initializeBoard(int *localState, int nRowsGlobal, int nColsGlobal, int currProcess, int nProcesses);

void sendGhostCells(int *localState, int nColsGlobal, int currProcess, int nProcesses, MPI_Datatype ghostCol);

void recvGhostCells(int *localState, int nColsGlobal, int currProcess, int nProcesses, MPI_Datatype ghostCol);
