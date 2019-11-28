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


//Main function
int main (int argc, char *argv[]){

    // Initialising mainand volatiles variables for compiler optimisation
    FILE *fp;
    FILE *fw;
    char * line_void = NULL;
    volatile int lines = 0;
    size_t len = 0;

    //Temp time variables
    volatile double t1;
    volatile double t2;

    //Total time variables
    double Tt1;
    double Tt2;

    Tt1 = getTime();
    //#######################################################################################################
    // Get coverage
    printf("Analysing coverage ...\n");
    t1 = getTime();
    fp = fopen("reads.fasta", "r"); //We open a fiirst time the reads.fasta file
    getline(&line_void, &len, fp); //we skip the first line (commentary) to skip to the first data line
    int coverage = getline(&line_void, &len, fp); //We save the line to count the chars numbers
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);
    fclose(fp); //We reset ou progress in the file

    //#######################################################################################################
    // Get lines number
    printf("Analysing line number ...\n");
    t1 = getTime();
    fp = fopen("reads.fasta", "r"); //We reopen the file after the reset
    struct statfs fsInfo = {0}; //We initialise a statfs struct
    int fd = fileno(fp); //We get the file info
    long optimalSize;

    struct stat buf;
    fstat(fd, &buf);
    long file_size = buf.st_size;

    if (fstatfs(fd, &fsInfo) == -1) {
        optimalSize = 4 * 1024 * 1024; //If we can't get the optimal buffer size for I/O, we use a balanced size
    } else {
        optimalSize = fsInfo.f_bsize; //We get the optimal buffer size for I/O defined by the system
    }

    volatile long size_counter = 0;
    char *BUFFER_SECONDARY = malloc(optimalSize); //We create the optimal buffer for I/O

    while(size_counter < file_size){ //We count the '/n' in the file, by intering on the buffer and on the file
        fread(BUFFER_SECONDARY, optimalSize, 1,fp);
        size_counter = size_counter + optimalSize;
        for(int i = 0; i < optimalSize ; i++){
            if(BUFFER_SECONDARY[i] == '\n'){
                lines++;
            }
        }
    }


    free(BUFFER_SECONDARY);
    fclose(fp); //We reset again our progress in the file
    t2 = getTime();
    //printf ("%d\n",lines);
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Regularizing values and creating BUFFER
    printf("Creating primary Buffer ...\n");
    t1 = getTime();
    int MAX_SIZE = lines/2; //We divise the total file line by 2 to get the data line number

    char **BUFFER_ONE = malloc(MAX_SIZE * sizeof(char*)); //We create the buffer with the info we had in the previous parts
    for (int i = 0; i < MAX_SIZE; i++){
        BUFFER_ONE[i] = malloc((coverage) * sizeof(char));
    }
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Buffering file content
    printf("Buffering file ...\n");
    t1 = getTime();
    fp = fopen("reads.fasta", "r");
    len = 0;
    volatile unsigned int counter = 0;
    getline(&line_void, &len, fp);
    while(getline(&BUFFER_ONE[counter], &len, fp) != -1){ //We iterate on the file and on the buffer to put the data in the buffer
        getline(&line_void, &len, fp);
        counter++;
    }
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Sorting the sequences by alphabetical order
    printf("Sorting buffer ...\n");
    t1 = getTime();
    qsort (BUFFER_ONE, MAX_SIZE, sizeof(char *), compare); //We use the compare function to sort the buffer. 
    //We use un quick sort because we don't have info on how well sorted is the file initially.

    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Deleting string without duplicate and saving strings with duplicate(s)
    printf("Processing ...\n");
    t1 = getTime();

    char **BUFFER_TWO = malloc(MAX_SIZE * sizeof(char*)); //We create a secondary BUFFER to save the results
    // We create the secondary buffer with the same size of the first one, because we can't know the real needed size
    volatile unsigned int scanner_cursor = 1; //Cursor of the first buffer (raw data)
    volatile unsigned int buffer_two_cursor = 0; //Cursor of the second buffer (results)
    volatile int zero_is_a_precedent_duplicate = 0; //This indicator is 0 if the precedent line is not a duplicate and 1 where it is. This is what we named the "duplicate alert"
    volatile int str_cmp_res = 0; //Initialising the variable saving the result of the string comparaison

    while (scanner_cursor < MAX_SIZE)
    {
        //We save the comparaison between the strings of the two cursors
        str_cmp_res = strcmp(BUFFER_ONE[scanner_cursor - 1], BUFFER_ONE[scanner_cursor]);

        //If we know that the first cursor don't containt a previous duplicate
        if(zero_is_a_precedent_duplicate == 0){

            //If the strings are the sames, then we save one of them into the second buffer, and we activate the previous duplicate alert
            if(str_cmp_res == 0){
                BUFFER_TWO[buffer_two_cursor] = BUFFER_ONE[scanner_cursor - 1];
                buffer_two_cursor++;
                zero_is_a_precedent_duplicate = 1;
            }
        
        //If we know that the first cursor containt a previous duplicate
        }else{

            //If the strings in the two cursor are differents we cancel the previous duplicate alert
            if(str_cmp_res != 0){
                zero_is_a_precedent_duplicate = 0;
                
            }
        }
        scanner_cursor++;
    }

    free(BUFFER_ONE);
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Writing file
    printf("Writing file ...\n");
    t1 = getTime();
    fw = fopen("reads_processed.txt","w"); //We write down the result buffer in the result file
    for(int i = 0; i < buffer_two_cursor; i++){
        fprintf (fw, "%s",BUFFER_TWO[i]);
    }
    fclose(fw);
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Closing everything
    free(BUFFER_TWO);

    Tt2 = getTime();
    printf ("Total time: %1.2lf sec\n",Tt2-Tt1);

}