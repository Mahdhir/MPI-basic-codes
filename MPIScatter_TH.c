#define SIZE 7
#include <mpi.h>
#include <stdio.h>
int main(int argc, char* argv[]) {
	int numtasks, rank, sendcount, recvcount, source;
	float sendbuf[SIZE][SIZE] = { {1.0, 2.0, 3.0, 4.0,5.0,6.0,7.0},{8.0,9.0,10.0,11.0,12.0,13.0,14.0},
		{15.0,16.0,17.0,18.0,19.0,20.0,21.0},{22.0,23.0,24.0,25.0,26.0,27.0,28.0} };
	float recvbuf[SIZE];
	MPI_Init(&argc, &argv); MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	source = 1;
	sendcount = SIZE; recvcount = SIZE;
	MPI_Scatter(sendbuf, sendcount, MPI_FLOAT, recvbuf, recvcount,
		MPI_FLOAT, source, MPI_COMM_WORLD);
	printf("rank= %d Results: ", rank);
	int length = sizeof(recvbuf) / sizeof(int);
	for (int i = 0; i < length; i++) {
		printf("%f ", recvbuf[i]);
	}
	MPI_Finalize();
}