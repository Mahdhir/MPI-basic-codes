#include <mpi.h>     // MPI
#include <stdio.h>   // printf
#include <stdlib.h>  // malloc, free, rand(), srand()
#include <time.h>    // time for random generator
#include <math.h>    // log2
#include <string.h>  // memcpy
#include <limits.h>  // INT_MAX

void powerOfTwo(int id, int numberProcesses);
void getInput(int argc, char* argv[], int id, int numProcs, int* arraySize);
void fillArray(int array[], int arraySize, int id);
void printList(int id, char arrayName[], int array[], int arraySize);
int compare(const void* a_p, const void* b_p);
int* merge(int half1[], int half2[], int mergeResult[], int size);
int* mergeSort(int height, int id, int localArray[], int size, MPI_Comm comm, int globalArray[]);


void powerOfTwo(int id, int numberProcesses) {
    int power;
    power = (numberProcesses != 0) && ((numberProcesses & (numberProcesses - 1)) == 0);
    if (!power) {
        if (id == 0) printf("number of processes must be power of 2 \n");
        MPI_Finalize();
        exit(-1);
    }
}


void getInput(int argc, char* argv[], int id, int numProcs, int* arraySize) {
    if (id == 0) {
        if (id % 2 != 0) {
            fprintf(stderr, "usage: mpirun -n <p> %s <size of array> \n", argv[0]);
            fflush(stderr);
            *arraySize = -1;
        }
        else if (argc != 2) {
            fprintf(stderr, "usage: mpirun -n <p> %s <size of array> \n", argv[0]);
            fflush(stderr);
            *arraySize = -1;
        }
        else if ((atoi(argv[1])) % numProcs != 0) {
            fprintf(stderr, "size of array must be divisible by number of processes \n");
            fflush(stderr);
            *arraySize = -1;
        }
        else {
            *arraySize = atoi(argv[1]);
        }
    }
    // broadcast arraySize to all processes
    MPI_Bcast(arraySize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // negative arraySize ends the program
    if (*arraySize <= 0) {
        MPI_Finalize();
        exit(-1);
    }
}

void fillArray(int array[], int arraySize, int id) {
    int i;
    srand(id + time(0));
    for (i = 0; i < arraySize; i++) {
        array[i] = rand() % 100; //INT_MAX
    }

}


void printList(int id, char arrayName[], int array[], int arraySize) {
    printf("Process %d, %s: ", id, arrayName);
    for (int i = 0; i < arraySize; i++) {
        printf(" %d", array[i]);
    }
    printf("\n");
}

int compare(const void* a_p, const void* b_p) {
    int a = *((int*)a_p);
    int b = *((int*)b_p);

    if (a < b)
        return -1;
    else if (a == b)
        return 0;
    else /* a > b */
        return 1;
}


int* merge(int half1[], int half2[], int mergeResult[], int size) {
    int ai, bi, ci;
    ai = bi = ci = 0;
    // integers remain in both arrays to compare
    while ((ai < size) && (bi < size)) {
        if (half1[ai] <= half2[bi]) {
            mergeResult[ci] = half1[ai];
            ai++;
        }
        else {
            mergeResult[ci] = half2[bi];
            bi++;
        }
        ci++;
    }
    // integers only remain in rightArray
    if (ai >= size) {
        while (bi < size) {
            mergeResult[ci] = half2[bi];
            bi++; ci++;
        }
    }
    // integers only remain in localArray
    if (bi >= size) {
        while (ai < size) {
            mergeResult[ci] = half1[ai];
            ai++; ci++;
        }
    }
    return mergeResult;
}


int* mergeSort(int height, int id, int localArray[], int size, MPI_Comm comm, int globalArray[]) {
    int parent, rightChild, myHeight;
    int* half1, * half2, * mergeResult;

    myHeight = 0;
    qsort(localArray, size, sizeof(int), compare); // sort local array
    half1 = localArray;  // assign half1 to localArray

    while (myHeight < height) { // not yet at top
        parent = (id & (~(1 << myHeight)));

        if (parent == id) { // left child
            rightChild = (id | (1 << myHeight));

            // allocate memory and receive array of right child
            half2 = (int*)malloc(size * sizeof(int));
            MPI_Recv(half2, size, MPI_INT, rightChild, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // allocate memory for result of merge
            mergeResult = (int*)malloc(size * 2 * sizeof(int));
            // merge half1 and half2 into mergeResult
            mergeResult = merge(half1, half2, mergeResult, size);
            // reassign half1 to merge result
            half1 = mergeResult;
            size = size * 2;  // double size

            free(half2);
            mergeResult = NULL;

            myHeight++;

        }
        else { // right child
           // send local array to parent
            MPI_Send(half1, size, MPI_INT, parent, 0, MPI_COMM_WORLD);
            if (myHeight != 0) free(half1);
            myHeight = height;
        }
    }

    if (id == 0) {
        globalArray = half1;   // reassign globalArray to half1
    }
    return globalArray;
}


int main(int argc, char** argv) {
    int numProcs, id, globalArraySize, localArraySize, height;
    int* localArray, * globalArray;
    double startTime, localTime, totalTime;
    double zeroStartTime, zeroTotalTime, processStartTime, processTotalTime;;
    int length = -1;
    char myHostName[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    MPI_Get_processor_name(myHostName, &length);

    powerOfTwo(id, numProcs);

    getInput(argc, argv, id, numProcs, &globalArraySize);

    height = log2(numProcs);
    globalArray= (int*)malloc(0);
    if (id == 0) {
        globalArray = (int*)malloc(globalArraySize * sizeof(int));
        fillArray(globalArray, globalArraySize, id);
        printList(id, "UNSORTED ARRAY", globalArray, globalArraySize);  // Line A
    }

    localArraySize = globalArraySize / numProcs;
    localArray = (int*)malloc(localArraySize * sizeof(int));
    MPI_Scatter(globalArray, localArraySize, MPI_INT, localArray, localArraySize, MPI_INT, 0, MPI_COMM_WORLD);

    startTime = MPI_Wtime();
    //Merge sort
    if (id == 0) {
        zeroStartTime = MPI_Wtime();
        globalArray = mergeSort(height, id, localArray, localArraySize, MPI_COMM_WORLD, globalArray);
        zeroTotalTime = MPI_Wtime() - zeroStartTime;
        printf("Process #%d of %d on %s took %f seconds \n",
            id, numProcs, myHostName, zeroTotalTime);
    }
    else {
        processStartTime = MPI_Wtime();
        mergeSort(height, id, localArray, localArraySize, MPI_COMM_WORLD, NULL);
        processTotalTime = MPI_Wtime() - processStartTime;
        printf("Process #%d of %d on %s took %f seconds \n",
            id, numProcs, myHostName, processTotalTime);
    }
    localTime = MPI_Wtime() - startTime;
    MPI_Reduce(&localTime, &totalTime, 1, MPI_DOUBLE,
        MPI_MAX, 0, MPI_COMM_WORLD);

    if (id == 0) {
        printf("Sorting %d integers took %f seconds \n", globalArraySize, totalTime);
        free(globalArray);
    }

    free(localArray);
    MPI_Finalize();
    return 0;
}