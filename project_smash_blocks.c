#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

/*  TO-DO List:
*       Have COL-size not hard coded
*       Make file read only to end of file
*       Make more dynamic array shit
*       Turn global variables into define
*/

int COL_SIZE = 4400; //Equal to number of rows (n rows, n keys)
int ROW_SIZE = 500; //Equal to number of columns (k columns)
double DIA = 0.000001; //dia for calculating neighbourhoods for blocks 
int MULTIPLIER = 1000000;

#define BLOCK_SIZE 4
#define MAX 4400

/*
* ElementInt - Used to store all data with coressponding row value
* Block - Used to store a block of 4 values
*/
struct ElementInt {
    int row_id;
    int value;
};

struct Block {
    int row_id[BLOCK_SIZE];
    int col_id_size;
    int* col_id;
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

int dataArray_size = 0;
struct ElementInt* dataArray;

void readKeys() {

    FILE *myFile;
    myFile = fopen("keys.txt", "r");

    if(myFile == NULL){
        fprintf(stderr, "File keys.txt not opened");
        return;
    }

    //read file into array
    for (int i = 0; i < COL_SIZE; i++)
    {
        //Dynamically increase key array to fit all values
        keyArray_size++;
        keyArray = realloc(keyArray, keyArray_size * sizeof(double));
        fscanf(myFile, "%lf", &keyArray[keyArray_size-1]);
    }

    fclose(myFile);
}

void readData() {

    FILE *myFile;
    myFile = fopen("data.txt", "r");

    if(myFile == NULL){
        fprintf(stderr, "File data.txt not opened");
        return;
    }

    //read file into array
    for (int i = 0; i < ROW_SIZE; i++)
    {
        for (int j = 0; j < COL_SIZE; j++)
        {
            dataArray_size++;
            dataArray = realloc(dataArray, dataArray_size * sizeof(struct ElementInt));

            double value;
            fscanf(myFile, "%lf,", &value);
            dataArray[dataArray_size-1].row_id = j;
            dataArray[dataArray_size-1].value = value * MULTIPLIER;
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
    struct ElementInt b[MAX];
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
* CREATE POWER SET AND CULL
* Time Complexity - O(n2^n)
* n - Number of items in set
*/
void createBlocks(int *set1, int set1_size, int *set2, int set2_size, int col_id){
    
    /*set_size of power set of a set with set_size
      n is (2**n -1)*/
    int set_size = set1_size + set2_size;
    unsigned int pow_set_size = pow(2, set_size);

    //Makes array of both sets combined
    int* set = malloc(set_size * sizeof(int));
    for(int i = 0; i < set_size; i++)
    {
        if(i < set1_size) set[i] = set1[i];
        else set[i] = set2[i-set1_size];
    }
 
    //Run counter 000..0 to 111..1
    for(int counter = 0; counter < pow_set_size; counter++){
      
        int storage_size = 0;
        int* storage = malloc(storage_size * sizeof(int));

        for(int j = 0; j < set_size; j++)
        {
            /* Check if jth bit in the counter is set
             If set then pront jth element from set */
            if(counter & (1<<j)){
                storage_size++;
                storage = realloc(storage, storage_size * sizeof(int));
                storage[storage_size-1] = set[j];
            }
        }

        //Check that subset has correct number of elements and an element from set1
        if(storage_size == BLOCK_SIZE){
            bool repeat = true;
            for (int i=0; repeat && i< storage_size ; i++) {
                for (int j=0; repeat && j < set1_size; j++){
                    if (storage[i] == set1[j]){

                        int sig = 0;
                        for(int k = 0; k < storage_size; k++){
                            sig += keyArray[storage[k]];
                        } 

                        //bool match = false;
                        // for (int k = 0; k < blockArray_size; k++)
                        // {
                        //     if (sig == blockArray[k].signature){
                        //         match = true;
                        //         blockArray[k].col_id_size++;
                        //         blockArray[k].col_id = realloc(blockArray[k].col_id, blockArray[k].col_id_size * sizeof(int));
                        //         blockArray[k].col_id[blockArray[k].col_id_size-1] = col_id;
                        //     }
                        // }
                        //if(!match) {

                            blockArray_size++;
                            blockArray = realloc(blockArray, blockArray_size * sizeof(struct Block));

                            struct Block currentBlock;
                            currentBlock.col_id_size = 1;
                            currentBlock.col_id = malloc(sizeof(int));
                            currentBlock.col_id[0] = col_id;
                            currentBlock.signature = 0;
                            // currentBlock.row_id = storage;
                            // currentBlock.signature = sig;

                            for(int k = 0; k < storage_size; k++){
                                currentBlock.row_id[k] = storage[k];
                                currentBlock.signature += keyArray[storage[k]];
                            }

                            //Add block to array of blocks
                            blockArray[blockArray_size-1] = currentBlock;
                        //}
                        
                        repeat = false;
                    }
                }
            }
        }
       
       free(storage);
    }

    free(set);
}

void createNeighbourhoods(struct ElementInt a[], int a_size, int col_id) {
    
    //Used for going through values within required bounds
    int lowerBound = 0;
    int upperBound = DIA * MULTIPLIER;

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
                for (int i = min; i <= currentHigh; i++)
                {
                    newNhood_size++;
                    newNhood = realloc(newNhood, newNhood_size * sizeof(int));
                    newNhood[newNhood_size-1] = a[i].row_id;
                }
                createBlocks(newNhood, newNhood_size, oldNhood, oldNhood_size, col_id);

            }
            else{

                if(oldNhood != NULL){ //Clear old Neighbourhood
                    free(oldNhood);
                    oldNhood_size = 0;
                    oldNhood = malloc(oldNhood_size * sizeof(int));
                }
                for (int i = min; i <= currentHigh; i++)
                {
                    oldNhood_size++;
                    oldNhood = realloc(oldNhood, oldNhood_size * sizeof(int));
                    oldNhood[oldNhood_size-1] = a[i].row_id;
                }
                createBlocks(oldNhood, oldNhood_size, NULL, 0, col_id);
            }

            storedHigh = currentHigh;
        }

        //Skip over chunks which won't have matches
        if ((currentHigh < a_size) && (upperBound < a[currentHigh+1].value)){
            upperBound = a[currentHigh+1].value;
            lowerBound = upperBound - (DIA * MULTIPLIER);
        }
        else if (lowerBound < a[currentLow].value){
            lowerBound = a[currentLow].value;
            upperBound = lowerBound + (DIA * MULTIPLIER);
        } 
        else { //If we cannot skip just increment by smallest amount (1 as numbers are ints)
            lowerBound += 1;
            upperBound += 1;
        } 
    }

    //Make sure memory is cleared once used
    free(oldNhood);
    free(newNhood);
}

void printBlock(struct Block block){
    for (int i = 0; i < block.col_id_size; i++)
    {
        printf("%d, ", block.col_id[i]);
    }
    printf(": ");
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        printf("%d ", block.row_id[i]);
    }
    printf("\t%f\n", block.signature);
}

int main() {

    blockArray = malloc(blockArray_size * sizeof(struct Block));
    keyArray = malloc(keyArray_size * sizeof(double));
    dataArray = malloc(dataArray_size * sizeof(struct ElementInt));

    readKeys();
    readData();

    //CHANGE 1 TO ROW_SIZE, ONCE TESTED
    for (int i = 0; i < 4; i++){

        int colArray_size = 0;
        struct ElementInt* colArray = malloc(colArray_size * sizeof(struct ElementInt));

        for (int j = 0; j < COL_SIZE; j++)
        {
            colArray_size++;
            colArray = realloc(colArray, colArray_size * sizeof(struct ElementInt));
            colArray[colArray_size-1] = dataArray[(COL_SIZE * i) + j];
        }

        sortData(colArray, colArray_size);
        createNeighbourhoods(colArray, colArray_size, i+1);
        free(colArray);
    }

    for (int i = 0; i < blockArray_size; i++)
    {
        printBlock(blockArray[i]); 
    }

    printf("%d\n", blockArray_size);
    
    free(blockArray);
    free(keyArray);
    free(dataArray);

    return 0;

}