#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>// necessary libraries to implement task

#define MATRIX_SIZE 100
int resMat[MATRIX_SIZE][MATRIX_SIZE];// i used global matrices because it is easier to store values in them
int resMatForDetach[MATRIX_SIZE][MATRIX_SIZE];



int **FirstMatrix;
int **SecondMatrix;
void allocateMatrices() { // this function is used to allocate memory for the two matrices
    FirstMatrix = (int **) malloc(MATRIX_SIZE * sizeof(int *));
    SecondMatrix = (int **) malloc(MATRIX_SIZE * sizeof(int *));

    for (int i = 0; i < MATRIX_SIZE; ++i) {
        FirstMatrix[i] = (int *) malloc(MATRIX_SIZE * sizeof(int));
       SecondMatrix[i] = (int *) malloc(MATRIX_SIZE * sizeof(int));

    }
}

typedef struct {// to pass multible arguments using one parameter in threads
    int startingRow;
    int endRow;
} ThreadARGS;

void generateFirstMatrix() {
    srand(time(NULL));

    int myId[] = {1, 2, 7, 5, 6};// my id numbers
    int size = sizeof(myId) / sizeof(myId[0]);// to find size of array
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            FirstMatrix[i][j] = myId[rand() % size];// this code will make sure for every index in the matrix it will be assigned with random number of my id
        }
    }

}

void generateSecondMatrix() {
    srand(time(NULL));

    int myId[] = {2, 4, 7, 1, 6, 5, 9};
    int size = sizeof(myId) / sizeof(myId[0]);

    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            SecondMatrix[i][j] = myId[rand() % size];
        }
    }

}
// this is a basic code to multiply two matrices and it has big o of n^3 run time !!
void multiplyMatrices(
        int result[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            result[i][j] = 0;
            for (int k = 0; k < MATRIX_SIZE; ++k) {
                result[i][j] += FirstMatrix[i][k] * SecondMatrix[k][j];
            }
        }
    }
}
/* function that divide the matrix multiplication by dividing thr numbers pf rows that can be multilblied in one time*/
void multiplyMatrixPartial(
        int result[MATRIX_SIZE][MATRIX_SIZE], int start_row, int end_row) {
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            result[i][j] = 0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                result[i][j] += FirstMatrix[i][k] * SecondMatrix[k][j];
            }
        }
    }
}

void multiplyMatricesByProcesses(int result[MATRIX_SIZE][MATRIX_SIZE], int numOfProcesses) {
    int FD[numOfProcesses][2];// pipes will be my ipc
    for (int i = 0; i < numOfProcesses; i++) {// creating pipes for each child proccess
        if (pipe(FD[i]) == -1) {
            printf("pipe failed\n");
            exit(EXIT_FAILURE);
        }
    }

    int rowsPerProc = MATRIX_SIZE / numOfProcesses;// to divide how many rows every proccess will take
    int remaining = MATRIX_SIZE % numOfProcesses;
    int *partialResult = (int *) malloc(rowsPerProc * MATRIX_SIZE * sizeof(int));// it will hold partial result of multiblcation

    for (int i = 0; i < numOfProcesses; i++) {// forking for number of proccesses
        pid_t pid = fork();
        if (pid == -1) {
            printf("fork error\n");//error handling
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            close(FD[i][0]);// closing read pipes when writing
            int startRow = i * rowsPerProc;
            int end_row;

            if (i == numOfProcesses - 1) {// deciding in which rows each procces will. beging to which row
                end_row = startRow + rowsPerProc + remaining;
            } else {
                end_row = startRow + rowsPerProc;
            }

            multiplyMatrixPartial(result, startRow, end_row);// calling partial mul function

            for (int row = 0; row < rowsPerProc; row++) {
                for (int col = 0; col < MATRIX_SIZE; col++) {
                    partialResult[row * MATRIX_SIZE + col] = result[startRow + row][col];
                }
            }

            write(FD[i][1], partialResult, rowsPerProc * MATRIX_SIZE * sizeof(int));// writing results in pipes
            close(FD[i][1]);
            exit(0);
        }
    }

    for (int i = 0; i < numOfProcesses; i++) {
        close(FD[i][1]);  // Close write end of the pipe

        read(FD[i][0], partialResult, rowsPerProc * MATRIX_SIZE * sizeof(int));// father proc reading values from child pipes
        close(FD[i][0]);

        // Copy the partial result back to the result matrix
        int startRow = i * rowsPerProc;
        for (int row = 0; row < rowsPerProc; row++) {
            for (int col = 0; col < MATRIX_SIZE; col++) {
                result[startRow + row][col] = partialResult[row * MATRIX_SIZE + col];
            }
        }
    }

    free(partialResult);

    for (int i = 0; i < numOfProcesses; i++) {// wait for sons procs
        wait(NULL);
    }

}

void printMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int areMatricesEquivalent(int matrix1[MATRIX_SIZE][MATRIX_SIZE], int matrix2[MATRIX_SIZE][MATRIX_SIZE]) {// to check if our methods are valid

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            if (matrix1[i][j] != matrix2[i][j]) {
                return 0; // Matrices are not equivalent
            }
        }
    }

    return 1; // Matrices are equivalent
}

void *matrixMulRoutine(void *arg) {// i used this routine for join threadsa only
    ThreadARGS *ARGS = (struct ThreadARGS *) arg;

    for (int i = ARGS->startingRow; i < ARGS->endRow; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            resMat[i][j] = 0;

            for (int k = 0; k < MATRIX_SIZE; k++) {
                resMat[i][j] += FirstMatrix[i][k] * SecondMatrix[k][j];
            }


        }
    }

    pthread_exit(NULL);


}
void *matrixMulRoutineDetach(void *arg) {// same logic but used for detached threads
    ThreadARGS *ARGS = (struct ThreadARGS *) arg;

    for (int i = ARGS->startingRow; i < ARGS->endRow; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            resMatForDetach[i][j] = 0;

            for (int k = 0; k < MATRIX_SIZE; k++) {
                resMatForDetach[i][j] += FirstMatrix[i][k] * SecondMatrix[k][j];
            }


        }
    }

    pthread_exit(NULL);


}


void
threadsMatrixMulJoinable(
        int numOfThreads) {
    pthread_t th[numOfThreads];
    ThreadARGS args[numOfThreads];// array of a struct used to save more than one argument for the thread routine
    int rows_per_thread = MATRIX_SIZE / numOfThreads;

    for (int i = 0; i < numOfThreads; i++) {
        args[i].startingRow=i*rows_per_thread;
        args[i].endRow=(i+1)*rows_per_thread;
        pthread_create(&th[i], NULL, matrixMulRoutine, (void *)&args[i]);// creating threads and assigning it routines

    }
    for (int i = 0; i < numOfThreads; i++) {
        if (pthread_join(th[i], NULL) != 0) {// joining threads
            perror("thread join process failed !\n");
            exit(EXIT_FAILURE);
        }
    }
}
void detachThreadsMul(int numOfThreads){// nearly the same logic for joined

    pthread_t th[numOfThreads];
    ThreadARGS args[numOfThreads];
    int rows_per_thread = MATRIX_SIZE / numOfThreads;
    pthread_attr_t attr;
    pthread_attr_init(&attr);// creating the detached attr
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (int i = 0; i < numOfThreads; i++) {
        args[i].startingRow=i*rows_per_thread;
        args[i].endRow=(i+1)*rows_per_thread;
        pthread_create(&th[i], NULL, matrixMulRoutineDetach, (void *)&args[i]);

    }
    usleep(10000);// to wait for threads to end excuction

    pthread_attr_destroy(&attr);// destroying the attr after we dont need it



    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("Detached threads method time: %fs  num of threads %d\n", elapsed_time,numOfThreads);

}

int main() {
    // genearte two random matrices
    allocateMatrices();
    generateFirstMatrix();
    generateSecondMatrix();
    int numdetach=6;
    detachThreadsMul(numdetach);
    int resultMatrix[MATRIX_SIZE][MATRIX_SIZE];
    // multiplying the easy way
    //struct timespec start_time, end_time;
    multiplyMatrices( resultMatrix);
   // printf("normal method time %fs , throuput %f \n",time,(1/time));
    int numOfThreads = 2;
    threadsMatrixMulJoinable(numOfThreads);

    int eqq = areMatricesEquivalent(resMat, resultMatrix);

    if (eqq)
        printf("Matching!!\n");
    else printf("no not matching\n");
    int numOfProcesses =2;// we will try first using 2 processes
    int resultMatrix2[MATRIX_SIZE][MATRIX_SIZE];
    struct timespec start_time, end_time;

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    multiplyMatricesByProcesses(resultMatrix2, numOfProcesses);
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double time=(end_time.tv_sec - start_time.tv_sec)+(end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("process method time %fs , throuput %f , number of processes %d\n",time,(1/time),numOfProcesses);

    eqq = areMatricesEquivalent(resultMatrix, resultMatrix2);
  //  printMatrix(resultMatrix);
  //  printf("\n");
    if (eqq)
        printf("Matching!!\n");
    else printf("no not matching\n");
    numOfProcesses = 2;// we will try first using 2 processes
    int resultMatrix3[MATRIX_SIZE][MATRIX_SIZE];

    multiplyMatricesByProcesses(resultMatrix3, numOfProcesses);
   // printMatrix(resultMatrix3);

    eqq = areMatricesEquivalent(resultMatrix, resultMatrix3);
    if (eqq)
        printf("Matching!!\n");
    else printf("no not matching\n");
    eqq = areMatricesEquivalent(resultMatrix, resMatForDetach);
    if (eqq)
        printf("Matching!!\n");
    else printf("no not matching\n");

    return 0;

}
