/*Project 5: Parallel Zip
// Expanding the my-zip and my-unzip from project 2 Unix utilities. 
//Sources: Using locks with multiple threads: https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/ 
// Using threads: https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf
// Creating multiple threads: https://www.geeksforgeeks.org/multithreading-in-c/ 
//Using locks is based on these chapters from course book: 
Chapter 28: https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf
Chapter 29: https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks-usage.pdf
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
//Used for locking operations and are based on https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf: 
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER; 
// This structure is also based on this: https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf: 
typedef struct {
    char *line; 
    int length; 
} myarg_t;


int countChar(char, char *);
void *readFile(void *);
void *writeToStdout(void *); 
// countChar function is based on this website: https://javatutoring.com/c-program-to-count-occurrences-of-character-in-string/

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
    pthread_t p;
    myarg_t args;
    FILE * file; 
    char *line=NULL;
    size_t len;
    int r;
            
        if ((file = fopen(arg, "r")) == NULL) {
            fprintf(stderr, "my-zip: cannot open file\n");
            exit(1);
        } else {
            do {
                    r= getline(&line, &len, file); 
                    if (r != -1) {
                        //Starting new thread for writing to stdout (this can be thought as child thread of the read file thread)
                        if ((args.line = malloc((strlen(line)+1)*sizeof(char))) == NULL) {
                            fprintf(stderr, "malloc failed");
                            pthread_mutex_unlock(&lock);
                            exit(1);
                        }
                        //Locking the creation of thread for writing in the stdout.
                        // Locking is based on this: https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/  and chapter 28 and 29 from coursebook:
                        // Chapter 28: https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf
                        //Chapter 29: https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks-usage.pdf
                        pthread_mutex_lock(&lock);
                        strcpy(args.line, line); 
                        args.length = r; 
                        pthread_create(&p,NULL, writeToStdout, &args);
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
    myarg_t *args = (myarg_t *) arg;  
    char *line = args->line;
    int r = args->length; 
    int counter=0; 
    int total=0;
    char character = line[total];
    while (total < r) {
        total += counter; 
        character = line[total];
        if (character == '\0' || total > r) {  
            break; 
        }
        counter = countChar(character, line);
        //Writing the integer in binary form and writing char simultaneously: https://stackoverflow.com/questions/10810593/two-consecutive-fwrites-operation  
        //Info about using fwrite(): https://lutpub.lut.fi/bitstream/handle/10024/162908/Nikula2021-COhjelmointiopasV21.pdf?sequence=1&isAllowed=y
        fwrite(&counter, sizeof(int), 1, stdout); 
        fwrite(&character, sizeof(char), 1, stdout);       
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    //Multiple threads are based on this website: https://www.geeksforgeeks.org/multithreading-in-c/ 
    pthread_t tid; 

    if (argc == 1) {
        fprintf(stderr, "pzip: file1 [file2 ...]\n");
        exit(1);
    } else if (argc >= 2) {
        //Creating one thread for each input file  is based on this: https://www.geeksforgeeks.org/multithreading-in-c/ 
        // and this:  https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf      
       
        //Creating one thread for each input file with exception that there is run only threads as many as CPUs at a time. 
        if (argc-1 <= get_nprocs_conf()) {
            for (int i=0; i < argc-1 && i <= get_nprocs_conf(); i++) {
                pthread_create(&tid, NULL, readFile, argv[i+1]);
                if (i > get_nprocs()) {
                    break; 
                }
            }
            pthread_exit(NULL);
        } else {
            for (int i=0; i < argc-1; i++) {
                pthread_create(&tid, NULL, readFile, argv[i+1]);
                if (i > get_nprocs_conf()) {
                    //waiting for last thread to be done before continuing
                    pthread_join(tid, NULL);
                    continue; 
                }
            }
            pthread_exit(NULL);
        }
    }
        return(0); 
    }