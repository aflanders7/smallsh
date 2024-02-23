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
    int stop = 0;
    size_t count = 0; // the line count
    size_t output_idx = 0;

    while (stop == 0) {
        ssize_t len = getline(&line, &n, stdin);
        if (len == -1) {
            if (feof(stdin)) { stop = 1; } // EOF is not defined by the spec. I treat it as "STOP\n"
            else err(1, "stdin");
        }
        if (strcmp(line, "STOP\n") == 0) { stop = 1; } // Normal exit

        for (size_t n = 0; n < len; ++n) {
            buffer1[count][n] = line[n]; // copy over to shared 2d unbounded buffer
        }

        count += 1;

        if (pthread_mutex_trylock(&mutex1) == 0 || stop == 1) { // need to update if stopping
            out_idx1 = count;
            pthread_mutex_unlock(&mutex1);
            pthread_cond_signal(&buf1_full);
        }
    }

    free(line);
    return NULL;
}

void *lineSeparator(void *args){
    int stop = 0;
    size_t count = 0; // the line count

    while (stop == 0) {
        pthread_mutex_lock(&mutex1);
        while (! (count < out_idx1)) { // no new info in buffer
            pthread_cond_wait(&buf1_full, &mutex1);
        }
        pthread_mutex_unlock(&mutex1);

        if (strcmp(buffer1[count], "STOP\n") == 0) { stop = 1; } // Normal exit

        size_t len = strlen(buffer1[count]);
        for (size_t n = 0; n < len; ++n) {
            buffer2[count][n] = (buffer1[count][n] == '\n') ? ' ' :
                                buffer1[count][n];
        }
        count += 1;
    }

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