#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

int blockArray_size = 0;
int* blockArray;

int* createBlocks(int* bigger_blocks_size, int* bigger_blocks){
    
	int i,j,counter;

	int numThreads = 5;

	int totalBlockIncrease[numThreads];
	int* totalBlockAdditions[numThreads];
    
	omp_set_num_threads(numThreads);
	#pragma omp parallel private(counter)
	{
        int blocks_size = 0;
		int* blocks = malloc(blocks_size * sizeof(int));
        
		#pragma omp for
		for(counter = 0; counter < 30; counter++){
            blocks_size++;
			blocks = realloc(blocks, blocks_size * sizeof(int));
            blocks[blocks_size-1] = blocks_size;
		}

		totalBlockIncrease[omp_get_thread_num()] = blocks_size;
		totalBlockAdditions[omp_get_thread_num()] = blocks;
		
    }

	for(i=0;i<numThreads;i++){
		int storedSize = *bigger_blocks_size;
        *bigger_blocks_size = storedSize + totalBlockIncrease[i];
        bigger_blocks = realloc(bigger_blocks, *bigger_blocks_size * sizeof(int));
        
		for(j=0;j<totalBlockIncrease[i];j++){
			bigger_blocks[storedSize+j] = totalBlockAdditions[i][j];
        }
	}
    
    //free(blocks);
	return bigger_blocks;
}

int* createNeighbourhoods(int* blocks_size, int* blocks) {
	
	int total_result_size = 0;
	int* total_result = malloc(total_result_size * sizeof(int));
	
	int j;
	for(j = 0; j < 5; j++){
		int i;
		int result_size = 0;
		int *result = malloc(result_size * sizeof(int));
		result = createBlocks(&result_size, result);
		for(i = 0; i < result_size; i++){
			total_result_size++;
			total_result = realloc(total_result, total_result_size * sizeof(int));
			total_result[total_result_size-1] = result[i];
		}
	}
	//blocks = createBlocks(blocks_size, blocks);

	*blocks_size = total_result_size;

	return total_result;
}

int main() {

	int i,j;
    
    int numThreads = 10;
    
    int totalBlockIncrease[numThreads];
    int* totalBlockAdditions[numThreads];
   
    omp_set_num_threads(numThreads);
	omp_set_nested(1); // Enable nested parallel threads
    #pragma omp parallel private(i,j)
    {

		int blocks_size = 0;
		int* blocks = malloc(blocks_size * sizeof(int));

		#pragma omp for
        for (i = 0; i < 100; i++){
			int mini_blocks_size = 0;
			int* mini_blocks = malloc(mini_blocks_size * sizeof(int));
            mini_blocks = createNeighbourhoods(&mini_blocks_size, mini_blocks);
			for(j = 0; j < mini_blocks_size; j++){
				blocks_size++;
				blocks = realloc(blocks, blocks_size * sizeof(int));
				blocks[blocks_size-1] = mini_blocks[i];
			}
        }
        
        totalBlockIncrease[omp_get_thread_num()] = blocks_size;
        totalBlockAdditions[omp_get_thread_num()] = blocks;
        
    }
    
    for(i=0;i<numThreads;i++){
        int storedSize = blockArray_size;
        blockArray_size += totalBlockIncrease[i];
        blockArray = realloc(blockArray, blockArray_size * sizeof(int));
    		
        for(j=0;j<totalBlockIncrease[i];j++){
            blockArray[storedSize+j] = totalBlockAdditions[i][j];
        }
    }

	for(i=0;i<blockArray_size;i++){
		printf("%d,",blockArray[i]);
	}
	
	printf("\n%d\n", blockArray_size);

    //free(blocks);

    free(blockArray);

	double a = 13.4;
	printf("%f",a-fmod(a,1));
	printf("%d",(int)a);

    return 0;

}
/*
int main()
{
	int Number_of_threads = 5;

	int i,j;

	int bigArray_size = 0;
	int* bigArray = malloc(bigArray_size * sizeof(int));

	int totalIncrease[Number_of_threads];
	int* totalAdditions[Number_of_threads];
	
	omp_set_num_threads(Number_of_threads);
	#pragma omp parallel private(i,j)
	{ 
		int miniArray_size = 0;
		int* miniArray = malloc(miniArray_size * sizeof(int));

		#pragma omp for
		for(i=0; i<10; i++){
		   
		   for(j=0; j < omp_get_thread_num(); j++){
			   miniArray_size++;
			   miniArray = realloc(miniArray, miniArray_size * sizeof(int));
			   miniArray[miniArray_size-1] = j;
			}
		}
		totalIncrease[omp_get_thread_num()] = miniArray_size;
		totalAdditions[omp_get_thread_num()] = miniArray; 
	}

	for(i=0;i<Number_of_threads;i++){
		int storedSize = bigArray_size;
   		bigArray_size += totalIncrease[i];
		bigArray = realloc(bigArray, bigArray_size * sizeof(int));
		
		for(j=0;j<totalIncrease[i];j++){
			bigArray[storedSize+j] = totalAdditions[i][j];
		}
	}

	for(i=0;i<bigArray_size;i++){
		printf("%d,",bigArray[i]);
	}
	
	printf("\n%d\n", bigArray_size);
}*/