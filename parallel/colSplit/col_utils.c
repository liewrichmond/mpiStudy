#include <stdlib.h>
#include <stdio.h>
#include "col_utils.h"

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
