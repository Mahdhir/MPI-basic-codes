#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


int main(int argc, char** argv) {

    int myid, nprocs, root=0, num_count=10;
    int* sendarray, * recv_buffer = NULL;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);


    if (myid == root) {
        recv_buffer = (int*)malloc(sizeof(int) * nprocs * num_count);
    }

    srand(time(NULL));

    sendarray = (int*)malloc(sizeof(int) * num_count);
    for (int i = 0; i < num_count; i++) {
        sendarray[i] = (rand() % 20) + myid*100;
    }

    MPI_Gather(sendarray, num_count, MPI_INT, recv_buffer, num_count, MPI_INT, root, MPI_COMM_WORLD);

    if (myid == root) {
        // print 5 values from each process from the receive buffer
        for (int i = 0; i < 5 * nprocs; i++) {
            printf("received buffer[%d]: %d\n",i, recv_buffer[i]);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

}