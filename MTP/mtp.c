#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

// buffer for input and line separator
char outbuf1[80];

// index input
int out_shared1 = 0; // using out shared index to keep track of if the buffer is full

// mutex
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

// conditionals
pthread_cond_t out_cond1 = PTHREAD_COND_INITIALIZER;

// buffer for line separator and plus sign
char outbuf2[80];

// index input
int in_shared2 = 0; // used with input
int out_shared2 = 0;

// mutex
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

// conditionals
pthread_cond_t in_cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t out_cond2 = PTHREAD_COND_INITIALIZER;


// buffer for plus sign and output
char outbuf3[80];

// index input
int in_shared3 = 0; // used with input
int out_shared3 = 0;

// mutex
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;

// conditionals
pthread_cond_t in_cond3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t out_cond3 = PTHREAD_COND_INITIALIZER;


void *getInput(void *args){
    char *line = NULL;

    size_t in_current_idx = 0;
    size_t in_max_idx = 0;
    size_t out_current_idx = 0;

    for (;;) {
        ssize_t len = getline(&line, &in_current_idx, stdin); // read data
        if (len == -1) {
            if (feof(stdin)) break; // EOF treated as "STOP\n"
            else err(1, "stdin");
        }
        if (strcmp(line, "STOP\n") == 0) break; // TODO Normal exit; do I need to return to exit?
        char c = line[in_current_idx++]; // increments after it's used
        c = c + 1;  // idk about this for input

        // write to the shared buffer
        outbuf1[out_current_idx] = c;

        // no in shared index, so skip for this thread
        // write to shared output index
        if (mutex_trylock(mutex1) == 0) {
            out_shared1 = out_current_idx;
            mutex_unlock(mutex1);
            cond_signal(out_cond1);
        }

        // check if buffer is full
        pthread_mutex_lock(&mutex1);
        while (++out_shared1 == 80) {
            pthread_mutex_unlock(&mutex1);
            sleep(1);
            pthread_mutex_lock(&mutex);
        }

    }
    free(line);
    return NULL;
}

void *lineSeparator(void *args){

    return NULL;
}

void *plusSign(void *args){

    return NULL;
}

void *Output(void *args){

    return NULL;
}

int main(void) {
    pthread_t tid;
    pthread_create(&tid, NULL, getInput, NULL);
    pthread_create(&tid, NULL, lineSeparator, NULL);
    pthread_create(&tid, NULL, plusSign, NULL);
    pthread_create(&tid, NULL, Output, NULL);
    pthread_join(tid, NULL);
}