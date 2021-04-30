void swap_col(int **curr, int **next, int nRows);

int **init2DArray(int *buffer, int nRowsGlobal, int nColsGlobal, int nProcesses);

int *initLocalBuffer(int nRowsGlobal, int nColsGlobal, int nProcesses);

int getCurrState(int currRow, int currCol, int nRows, int nCols, int **currState);

int isAlive(struct coord *neighbor, int nRows, int nCols, int **currState);
