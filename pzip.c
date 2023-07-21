//Project 5: Parallel Zip
// Expanding the my-zip and my-unzip from project 2 Unix utilities. 
//Sources: Using locks with multiple threads: https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/ 
// Using threads: https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

pthread_mutex_t lock;
typedef struct {
    char *line; 
    int length; 
} myarg_t;

// countChar function is based on this website: https://javatutoring.com/c-program-to-count-occurrences-of-character-in-string/
int countChar(char, char *);
void *readFile(void *);
void *writeToStdout(void *); 


int countChar(char character, char *str) {
    int count = 0; 
    for (int i=0; str[i]; i++ ) {
        if (str[i] == character) {
            count++; 
        } 
    }
    return count; 

}



//This function reads the file to be compressed and then calls function where the content is compressed
void *readFile(void *arg) {
    char *inputFileName =  (char *) arg; 
    pthread_t p;
    myarg_t args;
    FILE * file; 
    char *line=NULL;
    size_t len;
    int r;
            
        if ((file = fopen(inputFileName, "r")) == NULL) {
            fprintf(stderr, "my-zip: cannot open file\n");
            exit(1);
        } else {
            do {
                    r= getline(&line, &len, file); 
                    if (r != -1) {
                        //Starting new thread for writing to stdout
                        pthread_mutex_lock(&lock);

                        if ((args.line = malloc((strlen(line)+1)*sizeof(char))) == NULL) {
                            fprintf(stderr, "malloc failed");
                            exit(1);
                        }
                        strcpy(args.line, line); 
                        args.length = r; 
                        pthread_create(&p, NULL, writeToStdout, &args); 
                        pthread_join(p, NULL);
                        pthread_mutex_unlock(&lock);
                    }
                    }while(r > 1);
                    free(line);
                    fclose(file);
        }
        return NULL;

}

// Here is written the output to the stdout when first the function has called other function countChar 
// for counting the number of each character in line. 
void *writeToStdout(void *arg) { 
    //pthread_mutex_lock(&lock);
    myarg_t *args = (myarg_t *) arg;  
    //printf("%s", args.line);
    char *line = args->line;
    int r = args->length; 
    int counter=0; 
    int total=0;
    char character = line[total];
    while (total < r) {
        total += counter; 
        character = line[total];
        if (character == '\0') {
            break; 
        }
        counter = countChar(character, line);
        //Writing the integer in binary form and writing char simultaneously: https://stackoverflow.com/questions/10810593/two-consecutive-fwrites-operation  
        //Info about using fwrite(): https://lutpub.lut.fi/bitstream/handle/10024/162908/Nikula2021-COhjelmointiopasV21.pdf?sequence=1&isAllowed=y
        fwrite(&counter, sizeof(int), 1, stdout); 
        fwrite(&character, sizeof(char), 1, stdout);            
    }
    //pthread_mutex_unlock(&lock);
    return NULL;
}

int main(int argc, char *argv[]) {
    char *inputFileName;
    int i=1; 
    pthread_t thread[argc-1];

    
    if (argc == 1) {
        fprintf(stderr, "pzip: file1 [file2 ...]\n");
        exit(1);
    } else if (argc >= 2) {
        //Creating one thread for each input file         
        while (i < argc) {
            if ((inputFileName = (char *)malloc(strlen(argv[i]))) == NULL) {
                fprintf(stderr, "malloc failed"); 
                exit(1);
            }
            strcpy(inputFileName, argv[i]);
            
            pthread_create(&(thread[i-1]), NULL, &readFile, argv[i]);
            i++;    
        }
        pthread_join(thread[0], NULL); 
        pthread_join(thread[1], NULL);
        //free(inputFileName);
        /*for (int i=1; i < argc; i++) {
            strcpy(inputFileName, argv[i]);
            readFile(inputFileName);
            } 
        } */
    }
        return(0); 
    }