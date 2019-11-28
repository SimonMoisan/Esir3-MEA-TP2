#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include "klib/khash.h"

KHASH_SET_INIT_STR(str);

int masqueT = 0x10;
int masqueG = 0x04;
int masqueC = 0x02;

//Function for comparing two string (array of chars) used for the qsort function.
static int compare (const void *a, const void *b) 
{ 
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}

//Function used to get the current time
double getTime(void)
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (1.0e-6*t.tv_usec + t.tv_sec);
}


// A = 65 = 0100 0001
// T = 84 = 0101 0100
// C = 67 = 0100 0011
// G = 71 = 0100 0111
//          ...T .GC.
// T        0001 0000
// G        0000 0100
// C        0000 0010
// A         LE RESTE

//Writing a buffer in file file
void writingInFile(FILE *fw, char **buffer, unsigned long size){
    for(int i = 0; i < size; i++){
        fprintf (fw, "%s", buffer[i]);
    }
}


/**
void convertissor(const char *initial, const char *result){

    int initSize = sizeof(initial)/sizeof()


    return NULL;
}
**/

//Main function
int main (int argc, char *argv[]){

    // Variable definition
    FILE *fp_sort;
    FILE *fp_test;
    FILE *fw;
    volatile double t1;
    volatile double t2;
    struct statfs fsInfo_sort = {0}; //We initialise a statfs struct
    struct statfs fsInfo_test = {0}; //We initialise a statfs struct
    unsigned long optimalSize_sort;
    unsigned long optimalSize_test;
    long file_size_sort;
    long file_size_test;
    struct stat buf_sort;
    struct stat buf_test;
    int absent;
    int is_missing;
    volatile unsigned int cursorResult;


    // Determining optimal size I/O buffer with system help for sort_read.txt ################################################################
    printf("Determining optimal size buffer for I/O for sort_read.txt...\n");
    t1 = getTime();
    fp_sort = fopen("sort_reads.txt", "r"); //We open the sort file after
    fp_test = fopen("test_reads.txt", "r"); //We open the test file after
    
    int fd_sort = fileno(fp_sort); //We get the sort file info
    int fd_test = fileno(fp_test); //We get the sort file info

    fstat(fd_sort, &buf_sort);
    fstat(fd_test, &buf_test);
    
    file_size_sort = buf_sort.st_size;
    file_size_test = buf_test.st_size;

    //Getting optimal size for sort_reads
    if (fstatfs(fd_sort, &fsInfo_sort) == -1) {
        optimalSize_sort = 808 * 50; //If we can't get the optimal buffer size for I/O, we use a balanced size, but a multiple of 808
    } else {
        unsigned long sizeTempSort = fsInfo_sort.f_bsize; //We get the optimal buffer size for I/O defined by the system
        optimalSize_sort = (sizeTempSort / 808) * 808;
    }

    //Getting optimal size for test_reads
    if (fstatfs(fd_test, &fsInfo_test) == -1) {
        optimalSize_test = 808 * 50; //If we can't get the optimal buffer size for I/O, we use a balanced size, but a multiple of 808
    } else {
        unsigned long sizeTempTest = fsInfo_test.f_bsize; //We get the optimal buffer size for I/O defined by the system
        optimalSize_test = (sizeTempTest / 808) * 808;
    }

    t2 = getTime();
    printf("    - optimal buffer size for sort is %lu \n", optimalSize_sort);
    printf("    - optimal buffer size for test is %lu \n", optimalSize_test);
    printf(" - time: %1.2lf sec\n",t2-t1);


    volatile long size_counter = 0;
    char **BUFFER_sort = malloc(optimalSize_sort); //We create the optimal buffer for I/O
    for (int i = 0; i < optimalSize_sort; i++){
        BUFFER_sort[i] = malloc(101 * sizeof(char));
    }

    //Creating the hashmap and putting the line into it ################################################################
    printf("Putting sort_reads.txt into the hashmap ...\n");
    printf("test0");
    t1 = getTime();
    //We create the hashmap
    char s[128];
    printf("test1");
    khash_t(str) *h = kh_init(str);
    printf("test2");
    khint_t k;
    printf("test3");
    
    printf("test4");

    while(size_counter <= file_size_sort - optimalSize_sort){
        printf("test");
        fread(BUFFER_sort, optimalSize_sort, 1,fp_sort);
        printf("%lu", size_counter);
        size_counter = size_counter + optimalSize_sort;
    
        for(int i = 0; i < (sizeof(BUFFER_sort)/sizeof(*BUFFER_sort[0])) ; i++){
            
            k = kh_put(str, h, BUFFER_sort[i], &absent);
            if (absent){
                kh_key(h, k) = strdup(BUFFER_sort[i]);
            }
        }

    }

    free(BUFFER_sort);
    fclose(fp_sort);
    printf("    - hashmap size (number of different elements) is %d\n",kh_size(h));
    t2 = getTime();
    printf("    - time: %1.2lf sec\n",t2-t1);


    //#####################################################################################################
    //Reading test, testing if present and writing results
    printf("Reading test, testing if present and writing results");
    t1 = getTime();
    fp_test = fopen("test_reads.txt", "r");
    fw = fopen("exist_reads.txt","w");

    char **BUFFER_test = malloc(optimalSize_test); //We create the optimal buffer for I/o for the test file
        for (int i = 0; i < optimalSize_sort; i++){
        BUFFER_test[i] = malloc(101 * sizeof(char));
    }
    char **BUFFER_result = malloc(optimalSize_test); //We create the optimal buffer for I/o for the test file
        for (int i = 0; i < optimalSize_sort; i++){
        BUFFER_test[i] = malloc(101 * sizeof(char));
    }

    
    
    size_counter = 0;
    cursorResult = 0;
    while(size_counter <= file_size_test - optimalSize_test){
        fread(BUFFER_test, optimalSize_test, 1,fp_test);
        size_counter = size_counter + optimalSize_test;

        if(cursorResult >= optimalSize_test){
            writingInFile(fw, BUFFER_test, optimalSize_test);
            cursorResult = 0;    
        }

        for(int i = 0; i < optimalSize_test ; i = i++){
            is_missing = (*BUFFER_test[i] == kh_end(h));
            if (!is_missing){
                BUFFER_result[cursorResult] = BUFFER_test[i];
                cursorResult++;
            }
            
        }
    }
    t2 = getTime();
    printf("    - time: %1.2lf sec\n",t2-t1);

    //We free the buffers and we close the files
    free(BUFFER_test);
    free(BUFFER_result);
    fclose(fp_test);
    fclose(fw);

    //We free the heap used for the map
    for (k = 0; k < kh_end(h); ++k)
        if (kh_exist(h, k))
            free((char*)kh_key(h, k));
    kh_destroy(str, h);
    




}


