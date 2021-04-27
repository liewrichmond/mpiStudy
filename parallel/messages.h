#define ROOTPROCESS 0

void broadcastInitialState(int *globalState, int *localState, int nRows, int nCols, int nProcesses);

void receiveInitialState(int *buffer, int nRows, int nCols);

void initializeState(int** localState, int nRows, int nCols, int currProcess, int nProcesses);
