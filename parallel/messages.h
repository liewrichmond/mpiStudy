#include "mpi.h"

#define ROOTPROCESS 0
#define INITSTATE 0
#define RECVTOPROW 1
#define RECVBOTROW 2

void broadcastInitialState(int *globalState, int *localState, int nRows, int nCols, int nProcesses);

void receiveInitialState(int *buffer, int nRows, int nCols);

void initializeState(int *localState, int nRows, int nCols, int currProcess, int nProcesses);

int getParallelRankTop(int currRank, int nProcesses);

int getParallelRankBot(int currRank, int nProcesses);

int broadcastCurrState(int *currState, int nRows, int nCols, int currRank,  int nProcesses, MPI_Request *requests);

int recvCurrState(int *parallelTopRow, int *parallelBottomRow, int nCols, int currProcess, int nProcesses); 
