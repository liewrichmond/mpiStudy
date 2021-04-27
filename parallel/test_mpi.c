#include <stdlib.h>
#include <mpi.h>
#include <check.h>
#include "messages.h"

int nProcesses;
int currProcess;

// These don't work? Broken pipe error. Calling finalize multiple times breaks things;
// Call them in main() instead;
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

START_TEST(test_mpi_init)
{
    if(currProcess == ROOTPROCESS) {
        ck_assert_int_eq(currProcess,0);
        int recvData;
        MPI_Status status;
        for(int process = 1; process < nProcesses; process++) {
            MPI_Recv(&recvData, 1, MPI_INT, process, 0, MPI_COMM_WORLD, &status);
            ck_assert_int_eq(recvData, 100);
        }

    }else {
        int data = 100;
        MPI_Send(&data, 1, MPI_INT, ROOTPROCESS, 0, MPI_COMM_WORLD);
    } 
}
END_TEST

START_TEST(test_mpi_initial_state)
{
    int nRows = 4;
    int nCols = 4;
    int nRowsLocal = nRows/nProcesses;

    int *board = malloc(nRows * nCols * sizeof(*board));

    for (int i = 0; i < nRows * nCols; i++) {
        board[i] = i;
    }
    
    int *localState = malloc(nRowsLocal * nCols * sizeof(*localState));

    if(currProcess == ROOTPROCESS) {
       broadcastInitialState(board, localState, nRowsLocal, 4, nProcesses);
       for(int i = 0; i < nRowsLocal * nCols; i++) {
           ck_assert_int_eq(localState[i], i);
       }
    } else {
        receiveInitialState(localState, nRowsLocal, nCols);
        for(int i = 0; i < nRowsLocal*nCols; i++) {
            ck_assert_int_eq(localState[i], i + (nRowsLocal * nCols * currProcess));
        }
    }
}
END_TEST

Suite *makeSuite() {
    Suite *s = suite_create("MPI Test Suite");
    TCase *mpiTests = tcase_create("MPI Related Tests");
   
    tcase_add_test(mpiTests, test_test);
    tcase_add_test(mpiTests, test_mpi_init);
    tcase_add_test(mpiTests, test_mpi_initial_state);

    suite_add_tcase(s, mpiTests);

    return s;
}

int main() {
    setup();

    Suite *s = makeSuite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);

    teardown();
}
