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

// Initializing the hashmap from klib
KHASH_SET_INIT_STR(str);

// We use this mask to recognize nucleotides, the simplest way possible
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

// How we use the masks
// A = 65 = 0100 0001
// T = 84 = 0101 0100
// C = 67 = 0100 0011
// G = 71 = 0100 0111
//          ...T .GC.
// T        0001 0000
// G        0000 0100
// C        0000 0010
// A        Anything else

// This function convert/compress the nucleotides strings into another string, 4 times shorter
// A, T, C, G only 4 possibilities. So we can encode 1 nucleotide on only 2 bits.
// A char is 8 bits. So we use 1 char of the new string to encode 4 nucleotides
void Convertissor(const char * initial, unsigned char * result){

    int initSize = 100;
    volatile unsigned char newChar = 0;

    for(int i = 0; i < initSize - 4; i++){
        if(initial[i] & masqueT){     
            newChar = newChar | 0x03;
        }else if(initial[i+1] & masqueG){
            newChar = newChar | 0x02;
        }else if(initial[i+2] & masqueC){
            newChar = newChar | 0x01;
        }

        if((((i + 1) % 4) == 0) && i != 0){
            result[(i / 4)] = newChar;
            newChar = 0;
        }
        newChar <<= 2;
    }
    result[25] = '\0';
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
    char *line = NULL;
    //unsigned char lineConvert[26]; Useless if we don't use the convertissor function
    size_t len = 0;
    ssize_t read;

    // Creating the hashmap and putting the line into it ################################################################
    printf("Putting sort_reads.txt into the hashmap ...\n");
    fp_sort = fopen("sort_reads.txt", "r");
    t1 = getTime();
    //We create the hashmap
    
    char s[100];
    khash_t(str) *h = kh_init(str);
    khint_t k;

    // We read the entry file line after line. We put each line into the new hashmap
    while((read = getline(&line, &len, fp_sort)) != -1){

        //Here is the convertissor function, to optimize the size of each entry into the hashmap, desactivated here
        //Convertissor(line, lineConvert);

        // We put lineConvert in the place of line here, if we used the convertissor
        k = kh_put(str, h, line, &absent);
        if (absent){
            kh_key(h, k) = strdup(line);
        }
    }

    fclose(fp_sort);
    printf("    - hashmap size (number of different elements) is %d\n",kh_size(h));
    t2 = getTime();
    printf(" - time: %1.4lf sec\n",t2-t1);


    //#####################################################################################################
    //Reading test, testing if present and writing results
    printf("Reading test, testing if present and writing results\n");
    t1 = getTime();
    // We open at the same time the test file and the result file. We does't use buffer to respect the primary constraint of this work
    fp_test = fopen("test_reads.txt", "r");
    fw = fopen("exist_reads.txt","w");

    while((read = getline(&line, &len, fp_test)) != -1){

        // We convert the line to make it comparable to the others line converted in the hashmap,  desactivated here
        //Convertissor(line,lineConvert);

        // We put lineConvert in the place of line here, if we used the convertissor
        k = kh_get(str, h, line);
        is_missing = (k == kh_end(h));
        if (!is_missing){
            fprintf (fw, "%s", line);
        }
        
    }
    t2 = getTime();
    printf(" - time: %1.4lf sec\n",t2-t1);
    fclose(fp_test);
    fclose(fw);

    //We free the heap memory space used for the map
    for (k = 0; k <= kh_end(h); ++k)
        if (kh_exist(h, k))
            free((char*)kh_key(h, k));
    kh_destroy(str, h);
    
}


