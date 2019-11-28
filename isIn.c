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


char* convertissor(const char *initial, const char *result){

    int initSize = sizeof(initial)/sizeof(initial[0]);

    return NULL;
}

char* deconvertissor(const char *converti, const char *result){

    return NULL;
}



//Main function
int main (int argc, char *argv[]){

    // Variable definition
    FILE *fp_sort;
    FILE *fp_test;
    FILE *fw;
    volatile double t1;
    volatile double t2;
    int absent;
    int is_missing;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;


    //Creating the hashmap and putting the line into it ################################################################
    printf("Putting sort_reads.txt into the hashmap ...\n");
    fp_sort = fopen("sort_reads.txt", "r");
    t1 = getTime();
    //We create the hashmap
    char s[101];
    khash_t(str) *h = kh_init(str);
    khint_t k;

    while((read = getline(&line, &len, fp_sort)) != -1){
    
        k = kh_put(str, h, line, &absent);
        if (absent){
            kh_key(h, k) = strdup(line);
        }

    }

    fclose(fp_sort);
    printf("    - hashmap size (number of different elements) is %d\n",kh_size(h));
    t2 = getTime();
    printf(" - time: %1.2lf sec\n",t2-t1);


    //#####################################################################################################
    //Reading test, testing if present and writing results
    printf("Reading test, testing if present and writing results\n");
    t1 = getTime();
    fp_test = fopen("test_reads.txt", "r");
    fw = fopen("exist_reads.txt","w");

    while((read = getline(&line, &len, fp_test)) != -1){

        k = kh_get(str, h,line); 
        is_missing = (k == kh_end(h));
        if (!is_missing){
            fprintf (fw, "%s", line);
        } 
        
    }
    t2 = getTime();
    printf(" - time: %1.2lf sec\n",t2-t1);
    fclose(fp_test);
    fclose(fw);

    //We free the heap used for the map
    for (k = 0; k < kh_end(h); ++k)
        if (kh_exist(h, k))
            free((char*)kh_key(h, k));
    kh_destroy(str, h);
    




}


