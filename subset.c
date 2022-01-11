#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
 
// // Like "ENEMY enemies[100]", but from the heap
// ENEMY* enemies = malloc(100 * sizeof(ENEMY));

// // You can index pointers just like arrays.
// enemies[0] = CreateEnemy();

// // Make the array bigger
// enemies = realloc(enemies, 200 * sizeof(ENEMY));

// // Clean up when you're done.
// free(enemies);

//Time Complexity O(n2^n)
//where n is number in set
void printPowerSet(int *set1, int set1_size, int *set2, int set2_size, int subset_size)
{
    int set_size = set1_size + set2_size;

    /*set_size of power set of a set with set_size
      n is (2**n -1)*/
    unsigned int pow_set_size = pow(2, set_size);
    int counter, j;

    //Makes array of both sets combined
    int* set = malloc(set_size * sizeof(int));
    for(j = 0; j < set_size; j++)
    {
      if(j < set1_size) set[j] = set1[j];
      else set[j] = set2[j-set1_size];
    }
 
    /*Run from counter 000..0 to 111..1*/
    for(counter = 0; counter < pow_set_size; counter++){
      
      int storage_size = 0;
      int* storage = malloc(storage_size * sizeof(int));

      for(j = 0; j < set_size; j++)
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
       if(storage_size == subset_size){
          bool match = true;
          for (j=0; match && j< storage_size ; j++) {
            for (int k=0; match && k < set1_size; k++){
              if (storage[j] == set1[k]){
                for(int i = 0; i < storage_size; i++){
                  printf("%d ", storage[i]);
                }
                match = false;
              }
            }
          }
         
         printf("\n");
      }
       
       free(storage);
    }
    free(set);
}
 
/*Driver program to test printPowerSet*/
int main()
{
    int set1[] = {1140, 4087};
    int set2[] = {2083, 2173, 2214, 2425};
    printPowerSet(set1, 2, set2, 4, 4);

    int set3[] = {2083, 2173, 2214, 2425, 63};
    int set4[] = {};
    printPowerSet(set3, 5, NULL, 0, 4);
    return 0;
}

