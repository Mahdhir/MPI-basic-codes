#include <stdio.h>
#include <mpi.h>
#define NCOLS 5
int main(int argc, char** argv) {
	int i, j, k, l;
	int ierr, rank, size, root=0;
	float A[NCOLS];
	float Apart[NCOLS];
	float Bpart[NCOLS];
	float C[NCOLS];
	float A_exact[NCOLS];
	float B[NCOLS][NCOLS];
	float Cpart[1];

	/* Initiate MPI. */
	ierr = MPI_Init(&argc, &argv);
	ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	ierr = MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* Initialize B and C. */
	if (rank == root) {
		for (int row = 0; row < NCOLS; row++) {
			for (int col = 0; col < NCOLS; col++) {
				B[row][col] = row + col + 8;
			}
			C[row] = row + 9;
		}
	}

	/* Put up a barrier until I/O is complete */
	ierr = MPI_Barrier(MPI_COMM_WORLD);
	/* Scatter matrix B by rows. */
	ierr = MPI_Scatter(B, NCOLS, MPI_FLOAT, Bpart, NCOLS,
		MPI_FLOAT, root, MPI_COMM_WORLD);
	/* Scatter matrix C by columns */
	ierr = MPI_Scatter(C, 1, MPI_FLOAT, Cpart, 1, MPI_FLOAT,
		root, MPI_COMM_WORLD);
	/* Do the vector-scalar multiplication. */
	for (j = 0; j < NCOLS; j++)
		Apart[j] = Cpart[0] * Bpart[j];
	/* Reduce to matrix A. */
	ierr = MPI_Reduce(Apart, A, NCOLS, MPI_FLOAT,
		MPI_SUM, root, MPI_COMM_WORLD);

	if (rank == root) {
		printf("\nThis is the result of the parallel computation:\n\n");
		for (k = 0; k < NCOLS; k++) {
			printf("A[0]=%g\n", A[k]);
		}
		for (k = 0; k < NCOLS; k++) {
			A_exact[k] = 0.0;
			for (l = 0; l < NCOLS; l++) {
				A_exact[k] += C[l] * B[l][k];
			}
		}
		printf("\nThis is the result of the serial computation:\n\n");
		for (k = 0; k < NCOLS; k++) {
			printf("A_exact[0]=%g\n", A_exact[k]);
		}
	}
	MPI_Finalize();
}