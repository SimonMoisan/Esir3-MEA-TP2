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

// For an unsigned 
// 000 = 00 00 00 00
// 064 = 01 00 00 00
// 128 = 10 00 00 00 
// 192 = 11 00 00 00 

void Deconvertissor(const char *converti, char *result){

    int convertiSize = (sizeof(converti))/sizeof(converti[0]);

    char cursorChar = 0;

    for(int i = 0; i < convertiSize; i++){
        cursorChar = converti[i];
        for(int j = 0; j < 4; j++){
            if(cursorChar & 0xC0){
                result[i*4 + j] = 'T';
                cursorChar <<= 1;
            }else if(cursorChar & 0x80){
                result[i*4 + j] = 'G';
                cursorChar <<= 1;
            }else if(cursorChar & 0x40){
                result[i*4 + j] = 'C';
                cursorChar <<= 1;
            }else{
                result[i*4 + j] = 'A';
                cursorChar <<= 1;
            }
        }
    }

    result[convertiSize * 4] = '\n';
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
    unsigned char *lineConvert = malloc(26 * sizeof(unsigned char));
    size_t len = 0;
    ssize_t read;


    //Creating the hashmap and putting the line into it ################################################################
    printf("Putting sort_reads.txt into the hashmap ...\n");
    fp_sort = fopen("sort_reads.txt", "r");
    t1 = getTime();
    //We create the hashmap
    char s[26];
    khash_t(str) *h = kh_init(str);
    khint_t k;

   
    while((read = getline(&line, &len, fp_sort)) != -1){

        Convertissor(line, lineConvert);
        printf("\n##############VALUE: %s", lineConvert);   
        k = kh_put(str, h, lineConvert, &absent);
        if (absent){
            kh_key(h, k) = strdup(lineConvert);
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

        Convertissor(line,lineConvert);
        k = kh_get(str, h, lineConvert); 
        is_missing = (k == kh_end(h));
        if (!is_missing){
            fprintf (fw, "%s", line);
        } 
        
    }
    t2 = getTime();
    printf(" - time: %1.2lf sec\n",t2-t1);
    fclose(fp_test);
    fclose(fw);
    free(lineConvert);

    //We free the heap used for the map
    for (k = 0; k < kh_end(h); ++k)
        if (kh_exist(h, k))
            free((char*)kh_key(h, k));
    kh_destroy(str, h);
    
}


