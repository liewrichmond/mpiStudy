#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include "messages.h"
#include "col_utils.h"

int currProcess;
int nProcesses;

void setup(void) {
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &currProcess);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
}

void teardown(void) {
    MPI_Finalize();
}

START_TEST(test_test)
{
    ck_assert_int_eq(1,1);
}
END_TEST

START_TEST(testInitialState)
{
    setup();

    int nRows = 6;
    int nCols = 6;
    int nColsLocal = nCols / nProcesses;
    
    int *localState = initLocalBuffer(nRows, nCols, nProcesses);
    int **curr = init2DArray(localState, nRows, nCols, nProcesses);

    int *globalState = calloc(nRows * nCols, sizeof(*globalState));
    for (int i = 0; i< nRows * nCols; i++) {
        globalState[i] = i;
    }

    if(currProcess == 0) {
        sendInitialState(globalState, nRows, nCols, nProcesses);
    }

    recvInitialState(*curr, nRows, nCols, nProcesses);
    
    int expected;
    for(int i = 0; i < nRows; i++) {
        for(int j = 0; j < nColsLocal; j++) {
            expected = (currProcess * nColsLocal) + (i*nCols) + j;
            ck_assert_int_eq(curr[i][j], globalState[expected]);
        }
    }

}

START_TEST(testGhostCells)
{
    setup();

    int nRows = 6;
    int nCols = 6;
    int nColsLocal = nCols / nProcesses;
    
    int *localState = initLocalBuffer(nRows, nCols, nProcesses);
    int **curr = init2DArray(localState, nRows, nCols, nProcesses);
    
    for(int i = 0; i < nRows; i++) {
        for(int j = 0; j < nColsLocal; j++) {
            curr[i][j] = (currProcess * nColsLocal) + (i * nCols) + j;
        }
    }

    MPI_Datatype ghostCols_t = commitGhostCols(nRows, nColsLocal);
    sendGhostCells(*curr, nCols, currProcess, nProcesses, ghostCols_t);
    recvGhostCells(*curr, nCols, currProcess, nProcesses, ghostCols_t);
 
    if(currProcess == ROOTPROCESS) {
        int expectedLeft[] = {5,11,17,23,29,35};
        for(int i = 0; i < nRows; i++) {
            ck_assert_int_eq(curr[i][-1], expectedLeft[i]);
        }
        int expectedRight[] = {3,9,15,21,27,33};
        for(int i = 0; i < nRows; i++) {
            ck_assert_int_eq(curr[i][nColsLocal], expectedRight[i]);
        }
    }
    else {
        int expectedLeft[] = {2,8,14,20,26,32};
        for(int i = 0; i < nRows; i++) {
            ck_assert_int_eq(curr[i][-1], expectedLeft[i]);
        }
        int expectedRight[] = {0,6,12,18,24,30};
        for(int i = 0; i < nRows; i++) {
            ck_assert_int_eq(curr[i][nColsLocal], expectedRight[i]);
        }
    }

}

START_TEST(testInitializeBoard)
{
    setup();

    int nRows = 6;
    int nCols = 6;
    int nColsLocal = nCols / nProcesses;
    
    int *localState = initLocalBuffer(nRows, nCols, nProcesses);
    int **curr = init2DArray(localState, nRows, nCols, nProcesses);

    initializeBoard(*curr, nRows, nCols, currProcess, nProcesses);
    
    for(int i = 0; i < nRows; i++) {
        for(int j = -1; j < nColsLocal+1; j++) {
            ck_assert_int_eq(1, (curr[i][j] >= 0 && curr[i][j] <= 1));
        }
    }
}

Suite *makeSuite() {
    Suite *s = suite_create("MPI Test Suite");
    TCase *mpiTests = tcase_create("MPI Related Tests");
   
    tcase_add_test(mpiTests, test_test);
    tcase_add_test(mpiTests, testInitialState);
    tcase_add_test(mpiTests, testGhostCells);
    tcase_add_test(mpiTests, testInitializeBoard);
    suite_add_tcase(s, mpiTests);

    return s;
}

int main() {

    Suite *s = makeSuite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);

}
