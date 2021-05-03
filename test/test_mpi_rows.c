#include <stdlib.h>
#include <mpi.h>
#include <check.h>
#include "../src/parallel/rowSplit/messages.h"

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
    setup();
    if(currProcess == ROOTPROCESS) {
        ck_assert_int_eq(currProcess,0);
        int recvData;
        MPI_Status status;
        for(int process = 1; process < nProcesses; process++) {
            MPI_Recv(&recvData, 1, MPI_INT, process, 200, MPI_COMM_WORLD, &status);
            ck_assert_int_eq(recvData, 100);
        }

    }else {
        int data = 100;
        MPI_Send(&data, 1, MPI_INT, ROOTPROCESS, 200, MPI_COMM_WORLD);
    } 
}
END_TEST

START_TEST(test_mpi_initial_state)
{
    setup();
    int nRows = 4;
    int nCols = 4;
    int nRowsLocal = nRows/nProcesses;

    int *board = malloc(nRows * nCols * sizeof(*board));
    for (int i = 0; i < nRows * nCols; i++) {
        board[i] = i;
    }
    
    int *localState = malloc(nRowsLocal * nCols * sizeof(*localState));

    if(currProcess == ROOTPROCESS) {
       broadcastInitialState(board, localState, nRowsLocal, nCols, nProcesses);
       for(int i = 0; i < nRowsLocal * nCols; i++) {
           ck_assert_int_eq(localState[i], i);
       }
    } else {
        receiveInitialState(localState, nRowsLocal, nCols);
        for(int i = 0; i < nRowsLocal*nCols; i++) {
            ck_assert_int_eq(localState[i], i + (nRowsLocal * nCols * currProcess));
        }
    }

    free(board);
    free(localState);
}
END_TEST

START_TEST(testMpiBroadcastCurrState)
{
    setup();
    int nRows = 4;
    int nCols = 4;
    int nRowsLocal = nRows/nProcesses;
    
    int *localState = malloc(nRowsLocal * nCols * sizeof(*localState));
    int *paraStateBot = malloc(nCols * sizeof(*paraStateBot));
    int *paraStateTop = malloc(nCols * sizeof(*paraStateTop));

    for(int i = 0 ; i < nRowsLocal * nCols; i++) {
        localState[i] = i + (currProcess * nRowsLocal * nCols);
    }
    
    MPI_Request requests[5];
    MPI_Status status;
    int requestsMade = broadcastCurrState(localState, nRowsLocal, nCols, currProcess, nProcesses, requests);
    recvCurrState(paraStateTop, paraStateBot, nCols, currProcess, nProcesses);

    for(int i = 0; i < requestsMade; i++) {
        MPI_Wait(&requests[i], &status);
    }

    if(currProcess == ROOTPROCESS) {
        int expectedBot[] = {8,9,10,11};
        int expectedTop[] = {12,13,14,15};
        ck_assert_mem_eq(paraStateBot, expectedBot, nCols * sizeof(*expectedBot)); 
        ck_assert_mem_eq(paraStateTop, expectedTop, nCols * sizeof(*expectedTop)); 
    }
    else {
        int expectedBot[] = {0,1,2,3};
        int expectedTop[] = {4,5,6,7};
        ck_assert_mem_eq(paraStateBot, expectedBot, nCols * sizeof(*expectedBot)); 
        ck_assert_mem_eq(paraStateTop, expectedTop, nCols * sizeof(*expectedTop)); 
    }

    free(localState);
    free(paraStateBot);
    free(paraStateTop);
}
END_TEST

START_TEST(testParaRanks)
{
    setup();
    int topRank = getParallelRankTop(currProcess, nProcesses);
    int botRank = getParallelRankBot(currProcess, nProcesses);

    if(currProcess == ROOTPROCESS) {
        ck_assert_int_eq(1, topRank);
        ck_assert_int_eq(1, topRank);
    }
    else {
        ck_assert_int_eq(0, topRank);
        ck_assert_int_eq(0, botRank);
    }
}
END_TEST

Suite *makeSuite() {
    Suite *s = suite_create("MPI Test Suite");
    TCase *mpiTests = tcase_create("MPI Related Tests");
   
    tcase_add_test(mpiTests, test_test);
    tcase_add_test(mpiTests, test_mpi_init);
    tcase_add_test(mpiTests, test_mpi_initial_state);
    tcase_add_test(mpiTests, testMpiBroadcastCurrState);
    tcase_add_test(mpiTests, testParaRanks);

    suite_add_tcase(s, mpiTests);

    return s;
}

int main() {

    Suite *s = makeSuite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
}
