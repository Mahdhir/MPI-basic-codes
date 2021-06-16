#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define NCOLS 5

void print_matrix(char* prompt, float matrix[NCOLS][NCOLS]);

int main(int argc, char** argv) {

    int myrank, size, root=0;

   
    float A[NCOLS][NCOLS], Apart[NCOLS];
    float B[NCOLS][NCOLS], Bpart[NCOLS];
    float C[NCOLS][NCOLS];
    float A_exact[NCOLS][NCOLS];


    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    srand(time(NULL));
    for (int row = 0; row < NCOLS; row++) {
        for (int col = 0; col < NCOLS; col++) {
            B[row][col] = rand() % 5;
            C[row][col] = rand() % 3;
        }
    }

    MPI_Scatter(B, NCOLS, MPI_FLOAT, Bpart, NCOLS, MPI_FLOAT, root, MPI_COMM_WORLD);

    float total = 0.0;
    for (int i = 0; i < NCOLS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            total += Bpart[j] * C[j][i];
        }
        Apart[i] = total;
        total = 0.0;
    }


    MPI_Gather(Apart, NCOLS, MPI_FLOAT, A, NCOLS, MPI_FLOAT, root, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    if (myrank == root) {
        printf("\nThis is the result of the parallel computation:\n\n");
        print_matrix((char*)"A = ", A);

        for (int i = 0; i < NCOLS; i++) {
            for (int j = 0; j < NCOLS; j++) {
                A_exact[i][j] = 0.0;
                for (int k = 0; k < NCOLS; k++) {
                    A_exact[i][j] += B[i][k] * C[k][j];
                }
            }
        }
        printf("\nThis is the result of the serial computation:\n\n");
        print_matrix((char*)"A exact = ", A_exact);
    }

}

void print_matrix(char* prompt, float matrix[NCOLS][NCOLS])
{
    printf("\n%s\n", prompt);
    for (int i = 0; i < NCOLS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            printf(" %g", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}