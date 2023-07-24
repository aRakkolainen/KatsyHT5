#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
//void *arg is the name of the input file to be read with fread
void *printContent(void *arg) {
    FILE *file; 
    int number;
    char character; 
    if ((file = fopen(arg, "rb")) == NULL) {
                    fprintf(stderr, "my-unzip: cannot open file\n");
                    exit(1);
            }
            // Looping fread with while loop is based on this stackOverFlow discussion: https://stackoverflow.com/questions/15697783/how-does-fread-know-when-the-file-is-over-in-c
            while(fread(&number, sizeof(int), 1, file) == 1 && fread(&character, sizeof(char), 1, file) == 1) {
                for (int i=0; i < number; i++) {
                    printf("%c", character);
                }
            }
            fclose(file);
            printf("\n");
    return NULL; 
}

int main(int argc, char * argv[]) {
    /* Creating one thread for each input file i.e using multithreading is based on this: https://www.geeksforgeeks.org/multithreading-in-c/ 
    and this:  https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf   */   
       
    pthread_t tid; 
    if (argc == 1) {
        perror("my-unzip: file1 [file2 ...]\n");
        exit(1);
    } else {
        if (argc-1 < get_nprocs_conf()) {
            for (int i=0; i < argc-1 && i <= get_nprocs_conf(); i++) {
                pthread_create(&tid, NULL, printContent, argv[i+1]);
            }
        pthread_exit(NULL);
        } else {// If there are more input files than current system has CPUs, there is created threads as many as the CPUs and then is waited that 
        // the last one of those is done before is created the rest of needed threads. 
            for (int i=0; i < argc-1; i++) {
                pthread_create(&tid, NULL, printContent, argv[i+1]);
                if (i > get_nprocs_conf()) {
                    pthread_join(tid, NULL); //here is waited until the last thread is done before continuing with the loop
                    continue; 
                }
            }
        pthread_exit(NULL);
        }


        return(0);
    }
}