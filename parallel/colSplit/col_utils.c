#include <stdlib.h>
#include <stdio.h>
#include "../utils.h"
#include "col_utils.h"

void swap_col(int **curr, int **next, int nRows)
{
    int *temp;
    for(int i = 0; i < nRows; i++) {
        temp = *curr;
        curr[i] = next[i];
        next[i] = temp;
    }
}

int **init2DArray(int *buffer,int nRowsGlobal, int nColsGlobal, int nProcesses)
{
    int **twoDimArray = calloc(nRowsGlobal, sizeof(*twoDimArray));
    int nColsLocal = nColsGlobal / nProcesses; 
    for (int i = 0; i < nRowsGlobal; i++) {
        twoDimArray[i] = buffer + (i*(nColsLocal+2)) + 1;
    }

    return twoDimArray;
}

int *initLocalBuffer(int nRows, int nCols, int nProcesses)
{
    int nColsLocal = nCols / nProcesses;
    int *buffer = calloc((nRows * (nColsLocal + 2)), sizeof(*buffer));
    return buffer;
}

int getCurrState(int currRow, int currCol, int nRows, int nCols, int **currState)
{
    if(currCol < -1 || currCol > nCols) {
        exit(1);
    }
    //Wrap rows; negative column indexing is fine
    if(currRow < 0 || currRow >= nRows) {
        currRow = currRow < 0 ? nRows - 1 : 0;
    }
    return currState[currRow][currCol];
}

int isAlive(struct coord *neighbor, int nRows, int nCols, int **currState)
{
    return getCurrState(neighbor->row, neighbor->col, nRows, nCols, currState);
}
