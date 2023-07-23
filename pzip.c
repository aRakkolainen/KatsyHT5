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
    //char *inputFileName =  (char *) arg; 
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
                        //Starting new thread for writing to stdout
                        //pthread_mutex_lock(&lock);

                        if ((args.line = malloc((strlen(line)+1)*sizeof(char))) == NULL) {
                            fprintf(stderr, "malloc failed");
                            exit(1);
                        }
                        strcpy(args.line, line); 
                        args.length = r; 
                        pthread_create(&p, NULL, writeToStdout, &args); 
                        //pthread_mutex_unlock(&lock);
                        pthread_join(p, NULL);
                    }
                    }while(r > 1);
                    //pthread_join(p, NULL);
        
                    free(line);
                    fclose(file);
        }
        return NULL;

}

// Here is written the output to the stdout when first the function has called other function countChar 
// for counting the number of each character in line. 
void *writeToStdout(void *arg) { 
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
        pthread_mutex_lock(&lock);
        counter = countChar(character, line);
        //Writing the integer in binary form and writing char simultaneously: https://stackoverflow.com/questions/10810593/two-consecutive-fwrites-operation  
        //Info about using fwrite(): https://lutpub.lut.fi/bitstream/handle/10024/162908/Nikula2021-COhjelmointiopasV21.pdf?sequence=1&isAllowed=y
        fwrite(&counter, sizeof(int), 1, stdout); 
        fwrite(&character, sizeof(char), 1, stdout);  
        pthread_mutex_unlock(&lock);          
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    //char *inputFileName;
    //int i=1; 
    //int total = argc; 
    //pthread_t thread[argc-1];
    //Multiple threads are based on this website: https://www.geeksforgeeks.org/multithreading-in-c/ 
    pthread_t tid; 

    
    if (argc == 1) {
        fprintf(stderr, "pzip: file1 [file2 ...]\n");
        exit(1);
    } else if (argc >= 2) {
        //Creating one thread for each input file  is based on this: https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/  
        // and this:  https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf      
        /*while (i < total && i < get_nprocs()) {    
            pthread_create(&(thread[i-1]), NULL, &readFile, argv[i]);
            i++;    
        }
        for (int i=1; i < argc-1; i++) {
            pthread_join(thread[i], NULL);
        }*/
        //Creating one thread for each input file
        for (int i=0; i < argc-1 && i <= get_nprocs_conf(); i++) {
            pthread_create(&tid, NULL, readFile, argv[i+1]);
            if (i > get_nprocs_conf()) {
                pthread_join(tid, NULL); 
                pthread_create(&tid, NULL, readFile, argv[i+1]);
            }
        }
        pthread_exit(NULL);
        // So this almost works in parallel, except the order for handling the files is different every time... So the locking system doesn't work?
        // Miten sen nyt sais toimimaa siten, et se kirjoittaa yhden tiedoston sisällön kerrallaan ennen kuin seuraavan... 
    }
        return(0); 
    }