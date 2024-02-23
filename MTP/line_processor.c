#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 1000
#define LINE 50

// buffer for input and line separator
char buffer1[LINE][SIZE];
// index input
size_t out_idx1 = 0; // using out shared index to keep track of if the buffer is full
// mutex
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
// conditionals
pthread_cond_t buf1_full = PTHREAD_COND_INITIALIZER;


// buffer for line separator and plus sign
char buffer2[LINE][SIZE];
// index input
int out_idx2 = 0;
// mutex
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
// conditionals
pthread_cond_t buf2_full = PTHREAD_COND_INITIALIZER;


// buffer for plus sign and output
char buffer3[LINE][SIZE];
// index input
int out_idx3 = 0;
// mutex
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
// conditionals
pthread_cond_t buf3_full = PTHREAD_COND_INITIALIZER;


void *getInput(void *args){
    char *line = NULL;
    size_t n = 0;

    for (;;) {}

    free(line);
    return NULL;
}

void *lineSeparator(void *args){


    for (;;) {}

    return NULL;
}

void *plusSign(void *args){
    return NULL;
}

void *output(void *args){
    return NULL;
}

int main(void) {
    pthread_t in, ls, ps, op;
    pthread_create(&in, NULL, getInput, NULL);
    pthread_create(&ls, NULL, lineSeparator, NULL);
    pthread_create(&ps, NULL, plusSign, NULL);
    pthread_create(&op, NULL, output, NULL);
    pthread_join(in, NULL);
    pthread_join(ls, NULL);
    pthread_join(ps, NULL);
    pthread_join(op, NULL);
}