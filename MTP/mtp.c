#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

// buffer for input and line separator
char outbuf1[80];

// index input
int in_shared1 = 0; // dont need for step 1
int out_shared1 = 0;

// mutex
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

// conditionals
pthread_cond_t in_cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t out_cond1 = PTHREAD_COND_INITIALIZER;


char outbuf2[80];
int count2 = 0;

char outbuf3[80];
int count3 = 0;


void *getInput(void *args){
    char *line = NULL;

    size_t in_current_idx = 0;
    size_t in_max_idx = 0;
    size_t out_current_idx = 0;

    for (;;) {
        ssize_t len = getline(&line, &in_current_idx, stdin); // read data
        if (len == -1) {
            if (feof(stdin)) break; // EOF is not defined by the spec. I treat it as "STOP\n"
            else err(1, "stdin");
        }
        if (strcmp(line, "STOP\n") == 0) break; // TODO Normal exit; do I need to return to exit?
        char c = line[in_current_idx++]; // increments after it's used
        c = c + 1;  // idk about this for input
        outbuf1[out_current_idx] = c;

        // no in shared index, so skip for this thread
        // write to shared output index
        if (mutex_trylock(mutex1) == 0) {
            out_shared1 = out_current_idx;
            mutex_unlock(mutex1);
            cond_signal(out_cond1);
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