#include <mpi.h>     // MPI
#include <stdio.h>   // printf
#include <stdlib.h>  // malloc, free, rand(), srand()
#include <time.h>    // time for random generator
#include <math.h>    // log2
#include <string.h>  // memcpy
#include <limits.h>  // INT_MAX


int getInput(int argc, char* argv[]);
void fillArray(int array[], int arraySize);
void printList(char arrayName[], int array[], int arraySize);
void merge(int* arr, int l, int m, int r);
void merge_sort(int* arr, int l, int r);


int getInput(int argc, char* argv[]) {
    int arraySize = 0;
    if (argc != 2) {
        fprintf(stderr, "usage: mpirun <p> %s <size of array> \n", argv[0]);
        fflush(stderr);
        arraySize = -1;
    }
    else {
        arraySize = atoi(argv[1]);
    }
        return arraySize;
}

void fillArray(int array[], int arraySize) {
    int i;
    srand(time(NULL));
    for (i = 0; i < arraySize; i++) {
        array[i] = rand() % 100; //INT_MAX
    }

}

void printList(char arrayName[], int array[], int arraySize) {
    for (int i = 0; i < arraySize; i++) {
        printf(" %d", array[i]);
    }
    printf("\n");
}

void merge_sort(int* arr, int l, int r)
{
    if (l < r) {
        int m = l + (r - l) / 2;

        merge_sort(arr, l, m);
        merge_sort(arr, m + 1, r);

        merge(arr, l, m, r);
    }
}

void merge(int* arr, int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    int* L = (int*)malloc(sizeof(int) * n1);
    int* R = (int*)malloc(sizeof(int) * n2);

    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

 
    i = 0;
    j = 0;
    k = l;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    //Copy the remaining elements of L[], if there are any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

int main(int argc, char** argv) {
    int numProcs, globalArraySize;
    int* globalArray;
    double startTime, endTime;

    // In order to use an accurate time measurement
    MPI_Init(&argc, &argv);

    globalArraySize = getInput(argc, argv);

    globalArray = (int*)malloc(globalArraySize * sizeof(int));
    fillArray(globalArray, globalArraySize);

    startTime = MPI_Wtime();
    merge_sort(globalArray, 0, globalArraySize);
    endTime = MPI_Wtime();

    printf("Sorting %d integers took %f seconds \n", globalArraySize, endTime-startTime);

    MPI_Finalize();
    return 0;
}