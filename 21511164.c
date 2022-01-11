#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <math.h>
#include <sys/time.h>

#include <omp.h>

#define BLOCK_SIZE 4	//Total size of blocks
#define ROW_SIZE 500	//Equal to number of columns (k columns)
#define COL_SIZE 4400	//Equal to number of rows (n rows, n keys)
#define DIA 0.000001	//dia for calculating neighbourhoods for blocks 

/*
* STRUCTURES:
*
* ElementInt - Used to store all data with coressponding row value
*		row_id - Stores row number
*		value  - Stores raw data value divided by DIA to give integer value
*
* Block - Used to store a block of BLOCK_SIZE values
*		row_id - Stores each row number in block
*		col_id - Stores column block is from
*	 signature - Stores sum of row number's keys
*
*/
struct ElementInt {
    int row_id;
    int value;
};

struct Block {
    int row_id[BLOCK_SIZE];
    int col_id;
    double signature;
};

/*
* blockArray - Stores all blocks created
* keyArray   - Stores all keys from keys.txt
* dataArray  - Stores all data from data.txt
*/
int blockArray_size = 0;
struct Block* blockArray;

int keyArray_size = 0;
double* keyArray;

struct ElementInt dataArray[ROW_SIZE][COL_SIZE];

//Used for testing, prints all the contents of a Block structure
void printBlock(struct Block block){
    printf("%d: ", block.col_id);
	int i;
    for (i = 0; i < BLOCK_SIZE; i++)
    {
        printf("%d ", block.row_id[i]);
    }
    printf("\t%f\n", block.signature);
}

//Used for reading information from keys.txt into keyArray
void readKeys() {

    FILE *myFile;
    myFile = fopen("keys.txt", "r");

    if(myFile == NULL){ 
        fprintf(stderr, "File keys.txt not opened");
        return;
    }

    //read file into array
	int i;
    for (i = 0; i < COL_SIZE; i++)
    {
        //Dynamically increase key array to fit all values
        keyArray_size++;
        keyArray = realloc(keyArray, keyArray_size * sizeof(double));
        fscanf(myFile, "%lf", &keyArray[keyArray_size-1]);
    }

    fclose(myFile);
}

//Used for reading information from data.txt into dataArray
void readData() {

    FILE *myFile;
    myFile = fopen("data.txt", "r");

    if(myFile == NULL){
        fprintf(stderr, "File data.txt not opened");
        return;
    }

    //read file into array
	int i,j;
	for(i = 0; i < ROW_SIZE; i++)
	{
		for (j = 0; j < COL_SIZE; j++)
		{
			//Read data into unique spot based on 
			//position in row/column as an ElementInt

			double value;
			fscanf(myFile, "%lf,", &value);
			dataArray[i][j].row_id = j;
			dataArray[i][j].value = value / DIA;
		}
	}

    fclose(myFile);
}

/* 
* RADIX SORT DATA
* Time Complexity - O(nk)
* Space Complexity - O(n+k)
* n - Size of array, 4400
* k - Number of digits, 6
*/
void sortData(struct ElementInt *a, int n) {
    int i, m = 0, exp = 1;
    struct ElementInt b[COL_SIZE];

    for (i = 0; i < n; i++) {
        if (a[i].value > m)
           m = a[i].value;
    }
    while (m / exp > 0) {
        int box[10] = {
            0
        }
        ;
        for (i = 0; i < n; i++)
            box[a[i].value / exp % 10]++;
        for (i = 1; i < 10; i++)
            box[i] += box[i - 1];
        for (i = n - 1; i >= 0; i--)
            b[--box[a[i].value / exp % 10]] = a[i];
        for (i = 0; i < n; i++)
            a[i] = b[i];
        exp *= 10;
    }
}

/* 
* QUICK SORT BLOCKS
* Time Complexity - O(nlog(n))
* Space Complexity - O(log(n))
* n - Size of array, 4400
*/
void sortBlocks(struct Block* a, int n) {
    int i, j;
	double p;
	struct Block t;

    if (n < 2)
        return;
    p = a[n / 2].signature;
    for (i = 0, j = n - 1;; i++, j--) {
        while (a[i].signature < p)
            i++;
        while (p < a[j].signature)
            j--;
        if (i >= j)
            break;
        t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
    sortBlocks(a, i);
    sortBlocks(a + i, n - i);
}

/* 
* CREATE POWER SET AND CULL
* Time Complexity - O(n2^n)
* n - Number of items in set, varies 
*/
struct Block* createBlocks(int *set1, int set1_size, int *set2, int set2_size, int col_id, int* bigger_blocks_size, struct Block* bigger_blocks){
	
	/*
	* Set size of power set based on total set size
	* Set size = n
	* Power set size = 2**n -1
	*/
    int set_size = set1_size + set2_size;
    unsigned int pow_set_size = pow(2, set_size);

    //Makes array of both sets combined
	int set[set_size];
    int i,j,k,counter;
	for(i = 0; i < set_size; i++)
    {
        if(i < set1_size) set[i] = set1[i];
        else set[i] = set2[i-set1_size];
    }

	int numThreads = 10;

	//Used for merging blocks together after parallel region
	int totalBlockIncrease[numThreads];
	struct Block* totalBlockAdditions[numThreads];
    
	omp_set_num_threads(numThreads);
	#pragma omp parallel private(counter,i,j,k)
	{
		int blocks_size = 0;
		struct Block* blocks = malloc(blocks_size * sizeof(struct Block));
        
		#pragma omp for
		//Run counter 000..0 to 111..1
		for(counter = 0; counter < pow_set_size; counter++){

			bool match = true;
      
			int storage_size = 0;
			int* storage = malloc(storage_size * sizeof(int));

			for(j = 0; j < set_size; j++)
			{
				/* Check if jth bit in the counter is set
				 If set then pront jth element from set */
				if(counter & (1<<j)){
					storage_size++;
					if(storage_size > BLOCK_SIZE){
						match = false;
						break;
					}
                    storage =  realloc(storage, storage_size * sizeof(int));
					storage[storage_size-1] = set[j];
				}
			}

			//Check that subset has correct number of elements and an element from set1
			if(match && storage_size == BLOCK_SIZE){
				bool repeat = true;
				for (i=0; repeat && i < storage_size ; i++) {
					for (j=0; repeat && j < set1_size; j++){
						if (storage[i] == set1[j]){

							//If succesful then create block and add to array
							struct Block currentBlock;
							currentBlock.col_id = col_id;
							currentBlock.signature = 0;

							for(k = 0; k < storage_size; k++){
								currentBlock.row_id[k] = storage[k];
								currentBlock.signature += keyArray[storage[k]];
							}

                            blocks_size++;
                            blocks = realloc(blocks, blocks_size * sizeof(struct Block));
                            blocks[blocks_size-1] = currentBlock;
							repeat = false;
						}
					}
				}
			}

			free(storage);
		}

		//Store information from each thread
		totalBlockIncrease[omp_get_thread_num()] = blocks_size;
		totalBlockAdditions[omp_get_thread_num()] = blocks;
		
    }

	//Combine information from each thread 
	for(i=0;i<numThreads;i++){
		int storedSize = *bigger_blocks_size;
        *bigger_blocks_size = storedSize + totalBlockIncrease[i];
        bigger_blocks = realloc(bigger_blocks, *bigger_blocks_size * sizeof(struct Block));
        
		for(j=0;j<totalBlockIncrease[i];j++){
			bigger_blocks[storedSize+j] = totalBlockAdditions[i][j];
        }
	}

	return bigger_blocks;
}

struct Block* createNeighbourhoods(struct ElementInt a[], int a_size, int col_id, int* blocks_size, struct Block* blocks) {
    
    int total_result_size = 0;
	struct Block* total_result = malloc(total_result_size * sizeof(struct Block));
	
	//Used for going through values within required bounds
    int lowerBound = 0;
    int upperBound = 1;

    //Used for finding at what point in the array we are searching
    int currentLow = 0;
    int currentHigh = 0;

    //Used to skip over chunks of data, where there is no information
    int storedHigh = 0;

    //Used for finding if we are adding new values to an already existing neighbourhood
    int newNhood_size = 0;
    int oldNhood_size = 0;
    int* newNhood = malloc(newNhood_size * sizeof(int));
    int* oldNhood = malloc(oldNhood_size * sizeof(int));

    while (lowerBound <= a[a_size-1].value){

        //Change where we are searching in the array based on the bounds we are looking for
        while (a[currentLow].value < lowerBound){
            currentLow++;
        }
        while (a[currentHigh].value <= upperBound){
            currentHigh++;
            if (currentHigh == a_size) break;
        }
        currentHigh--;

        //Produce a neighbourhood if there are enough values within the bounds
        if((currentHigh-currentLow+1) >= 4){

            int min = fmax(currentLow,storedHigh+1);

            if ((currentHigh-currentLow+1)!=(currentHigh-fmax(currentLow,storedHigh)+1)){

                /*Case where there will be new elements added
                to an existing neighbourhood (must include at least one from new)*/
                if(newNhood != NULL){ 
                    free(newNhood);
                    newNhood_size = 0;
                    newNhood = malloc(newNhood_size * sizeof(int));
                }
                int i,j;
				for (i = min; i <= currentHigh; i++)
                {
                    newNhood_size++;
                    newNhood = realloc(newNhood, newNhood_size * sizeof(int));
                    newNhood[newNhood_size-1] = a[i].row_id;
                }
				
				//Add produced blocks to total blocks array to be returned
				int result_size = 0;
				struct Block* result = malloc(result_size * sizeof(struct Block));
                result = createBlocks(newNhood, newNhood_size, oldNhood, oldNhood_size, col_id, &result_size, result);
				for(i = 0; i < result_size; i++){
					total_result_size++;
					total_result = realloc(total_result, total_result_size * sizeof(struct Block));
					total_result[total_result_size-1] = result[i];
				}
            }
            else{

                if(oldNhood != NULL){ //Clear old Neighbourhood
                    free(oldNhood);
                    oldNhood_size = 0;
                    oldNhood = malloc(oldNhood_size * sizeof(int));
                }
				int i,j;
                for (i = min; i <= currentHigh; i++)
                {
                    oldNhood_size++;
                    oldNhood = realloc(oldNhood, oldNhood_size * sizeof(int));
                    oldNhood[oldNhood_size-1] = a[i].row_id;
                }

				//Add produced blocks to total blocks array to be returned
				int result_size = 0;
				struct Block* result = malloc(result_size * sizeof(struct Block));
                result = createBlocks(oldNhood, oldNhood_size, NULL, 0, col_id, &result_size, result);
				for(i = 0; i < result_size; i++){
					total_result_size++;
					total_result = realloc(total_result, total_result_size * sizeof(struct Block));
					total_result[total_result_size-1] = result[i];
				}
				storedHigh = currentHigh;
            }

            storedHigh = currentHigh;
        }

        //Skip over chunks which won't have matches
        if ((currentHigh < a_size) && (upperBound < a[currentHigh+1].value)){
            upperBound = a[currentHigh+1].value;
            lowerBound = upperBound - 1;
        }
        else if (lowerBound < a[currentLow].value){
            lowerBound = a[currentLow].value;
            upperBound = lowerBound + 1;
        } 
        else { //If we cannot skip just increment by smallest amount (1 as numbers are ints)
            lowerBound += 1;
            upperBound += 1;
        } 
    }

    //Make sure memory is cleared once used
    free(oldNhood);
    free(newNhood);

	//Return all blocks
	*blocks_size = total_result_size;
	return total_result;
}

void findCollisions(struct Block* blocks, int blocks_size){

	int i,j,k;
	int collisionCount = 10;

	int numThreads = 10;
    
	//Parallel region adds each collisionCount to return a total number
	omp_set_num_threads(numThreads);
	#pragma omp parallel firstprivate(i,j,k) reduction(+:collisionCount)
	{
        
		#pragma omp for schedule(static)
		for(i = 0; i < blocks_size; i++){

			//Intialises collision set with one block
			int collision_size = 1;
			struct Block* collision = malloc(collision_size * sizeof(struct Block));
			collision[0] = blocks[i];
			
			for(j=i+1; j < blocks_size; j++){

				bool col_match = false;

				//Same signature
				if (blocks[i].signature == blocks[j].signature){

					//Checks they dont have same col_id
					for(k=0; k < collision_size; k++){ 
						if(blocks[j].col_id == collision[k].col_id){
							col_match = true;
							break;
						}
					}
					if (!col_match){
						collision_size++;
						collision = realloc(collision, collision_size * sizeof(struct Block));
						collision[collision_size-1] = blocks[j];
					}
				}
				else if (!col_match){

					//Prints entire collision if there is at two blocks
					if(collision_size > 1){ 
						printf("%f -", collision[0].signature);
						for(k=0; k < collision_size; k++){
							printf(" %d,", collision[k].col_id);
						}
						printf("\n");
						collisionCount++;
					}

					//Frees up memory as information has been printed
					free(collision);
					i += collision_size-1;
					break;
				}
			}
		}
	}

	//Display total collision count
	printf("\nTotal Collisions: %d\n", collisionCount);
}

int main() {

	//Intialise time structures to measure entire time taken for program
	struct timeval start, end;
	gettimeofday(&start, NULL);

    blockArray = malloc(blockArray_size * sizeof(struct Block));
    keyArray = malloc(keyArray_size * sizeof(double));

    //Read information from files
	readKeys();
    readData();

	int i,j;

	int numThreads = 10;
    
	//Used for merging blocks together after parallel region
    int totalBlockIncrease[numThreads];
    struct Block* totalBlockAdditions[numThreads];
    
    omp_set_num_threads(numThreads);
	omp_set_nested(1); // Enable nested parallel threads
    #pragma omp parallel private(i)
    {
		int blocks_size = 0;
		struct Block* blocks = malloc(blocks_size * sizeof(struct Block));
        
        #pragma omp for schedule(dynamic,5)
        for (i = 0; i < 500; i++){

			//Sorts column before looking for neighbourhoods
			sortData(dataArray[i], COL_SIZE);

			//For each column add blocks produced to master thread
			int mini_blocks_size = 0;
			struct Block* mini_blocks = malloc(mini_blocks_size * sizeof(struct Block));
            mini_blocks = createNeighbourhoods(dataArray[i], COL_SIZE, i+1, &mini_blocks_size, mini_blocks);
			for(j = 0; j < mini_blocks_size; j++){
				blocks_size++;
				blocks = realloc(blocks, blocks_size * sizeof(struct Block));
				blocks[blocks_size-1] = mini_blocks[j];
			}
            //Used for testing to display what column each thread is working on
			printf("%d(%d) - %d\n",i, omp_get_thread_num(), blocks_size);
        }
        
        //Store information from each thread
		totalBlockIncrease[omp_get_thread_num()] = blocks_size;
        totalBlockAdditions[omp_get_thread_num()] = blocks;
        
    }

	//Combine information from each thread 
    for(i=0;i<numThreads;i++){
        int storedSize = blockArray_size;
        blockArray_size += totalBlockIncrease[i];
        blockArray = realloc(blockArray, blockArray_size * sizeof(struct Block));
    		
        for(j=0;j<totalBlockIncrease[i];j++){
            blockArray[storedSize+j] = totalBlockAdditions[i][j];
        }
    }

	//Orders array of blocks and sorts through them
	sortBlocks(blockArray, blockArray_size);
	findCollisions(blockArray, blockArray_size);

	//Find time taken and print
	gettimeofday(&end, NULL);
	double delta = ((end.tv_sec  - start.tv_sec) * 1000000u +
			end.tv_usec - start.tv_usec) / 1.e6;

	printf("Time taken = %12.10f\n",delta);
    
    //Free up memory
	free(blockArray);
    free(keyArray);

    return 0;

}