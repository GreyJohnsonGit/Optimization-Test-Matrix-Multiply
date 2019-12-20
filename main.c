#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 1
#endif

#define MATRIX_SIZE 8192
#define BLOCK_SIZE 64
#define THREADS 6
#define BLOCKS ((MATRIX_SIZE*MATRIX_SIZE)/(BLOCK_SIZE*BLOCK_SIZE))
#define BLOCKS_PER_ROW (MATRIX_SIZE/BLOCK_SIZE)

#define POPULATE(matrix, size) \
for(int i = 0; i < size; i++) {\
	for(int j = 0; j < size; j++) {\
		matrix[i][j] = (double)rand();\
	}\
}\

//Define arrays
static double A[MATRIX_SIZE][MATRIX_SIZE];
static double B[MATRIX_SIZE][MATRIX_SIZE];
static double C[MATRIX_SIZE][MATRIX_SIZE];

struct threadInput {
	int beginRow;
	int beginCol;
	int endRow;
	int endCol;
};
void* parralized_multArray(void *in);

static int threads = THREADS;

int main() {

	pthread_t threads[THREADS];
	struct threadInput inputs[THREADS];
	for(int i = 0; i < THREADS; i++) {
		inputs[i].beginRow = i*BLOCKS/THREADS/BLOCKS_PER_ROW;
		inputs[i].beginCol = i*BLOCKS/THREADS%BLOCKS_PER_ROW;
		inputs[i].endRow = (i+1)*BLOCKS/THREADS/BLOCKS_PER_ROW;
		inputs[i].endCol = (i+1)*BLOCKS/THREADS%BLOCKS_PER_ROW;
		//printf("Begin: %d, %d\t\t| End: %d, %d\n", inputs[i].beginRow, inputs[i].beginCol, inputs[i].endRow, inputs[i].endCol);
	}

	//Poplate Arrays
	POPULATE(A, MATRIX_SIZE);
	POPULATE(B, MATRIX_SIZE);

	struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);
	for(int i = 0; i < THREADS; i++) {
		pthread_create(threads+i, NULL, &parralized_multArray, (inputs+i));
	}
	for(int i = 0; i < THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
	clock_gettime(CLOCK_REALTIME, &end);

	printf("Execution Time: %lfs\n", (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec)/1e9f);

	return C[12][12];

}

void* parralized_multArray(void *in){
	struct threadInput *input = in;
	struct {
		int row;
		int col;
	} loc;
	loc.row = input->beginRow;
	loc.col = input->beginCol;
	while(loc.row != input->endRow || loc.col != input->endCol) {
		for(int k = 0; k < MATRIX_SIZE; k++) {
			for(int i = loc.col*BLOCK_SIZE; i < (loc.col+1)*BLOCK_SIZE; i++) {
				for(int j = loc.row*BLOCK_SIZE; j < (loc.row+1)*BLOCK_SIZE; j++) {
					C[i][j] += A[i][k] + B[k][j];
				}
			}
		}
		loc.col++;
		if(loc.col == BLOCKS_PER_ROW) {
			loc.col = 0;
			loc.row++;
		}
	}

	for(int k = input->beginRow; k < input->endCol; k++) {
		for(int i = 0; i < MATRIX_SIZE; i++) {
			for(int j = 0; j < MATRIX_SIZE; j++) {
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	//printf("Thread %d finished\n", threads--);
	pthread_exit(NULL);
	return NULL;
}