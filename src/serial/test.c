#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

//int testMPI() {
//    int a = 100;
//    MPI_Init(NULL, NULL);
//    printf("%d\n", a);
//    MPI_Comm_rank(MPI_COMM_WORLD, &a);
//    printf("%d\n", a);
//    MPI_Finalize();
//    return 0;
//}
//
//
void sayHelloSingleProcess() {
    printf("Hello from a Single Process\n");
}

void sayHelloMPI()
{
    int currProcess, nProcesses;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &currProcess);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    
    printf("Hello from process %d out of %d processes\n", currProcess, nProcesses);
}

int makeRand() {
    
    srand(time(NULL));
    return rand();
}

void testStatus() {
    int rank;
    
    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0) {
        int nNumbers = 10;
        int a[nNumbers];
        for (int i = 0; i < nNumbers; i++) {
            a[i] = i + 20;
            printf("%d, ", a[i]);
        }
        printf("\n");
        MPI_Send(a, nNumbers, MPI_INT, 1, 10, MPI_COMM_WORLD);
    }
    else if(rank == 1){
        int b[300];
        MPI_Status status;
        MPI_Recv(b, 300, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int size;
        MPI_Get_count(&status, MPI_INT, &size);
        for (int j = 0 ; j < size; j++) {
            printf("%d, ", b[j]);
        }
        
    }

    MPI_Finalize();
}

int main() {
    //testStatus();
    //sayHelloMPI();
    //MPI_Finalize();
}
