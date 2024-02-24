#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 1000
#define LINE 50

// poison pull
char *str = "ğğ";

// buffer for input and line separator
// char *buffer1 = malloc(LINE*SIZE*sizeof(char));


// char buffer1[LINE][SIZE];
char (*buffer1)[SIZE];
// index input
size_t out_idx1 = 0; // using out shared index to keep track of if the buffer is full
// mutex
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
// conditionals
pthread_cond_t buf1_full = PTHREAD_COND_INITIALIZER;


// buffer for line separator and plus sign
char (*buffer2)[SIZE];
// index input
size_t out_idx2 = 0;
// mutex
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
// conditionals
pthread_cond_t buf2_full = PTHREAD_COND_INITIALIZER;


// buffer for plus sign and output
char (*buffer3)[SIZE];
// index input
size_t out_idx3 = 0;
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
    char buffer[1000];

    while (stop == 0) {
        ssize_t len = getline(&line, &n, stdin);
        if (len == -1) {
            if (feof(stdin)) { stop = 1; } // EOF is not defined by the spec. I treat it as "STOP\n"
            else err(1, "stdin");
        }
        if (strcmp(line, "STOP\n") == 0) {
            stop = 1;
            strcpy(buffer1[count], str);
        } // Normal exit

        else {
            output_idx = 0;
            for (size_t n = 0; n < len; ++n) {
                buffer1[count][output_idx] = line[n];
                output_idx++;
            }
        }

        count += 1;

        if (pthread_mutex_trylock(&mutex1) == 0 || stop == 1) { // need to update if stopping
            out_idx1 = count;
            pthread_mutex_unlock(&mutex1);
            pthread_cond_signal(&buf1_full);
        }
    }
    pthread_cond_signal(&buf1_full);

    free(line);
    return NULL;
}

void *lineSeparator(void *args){
    int stop = 0;
    size_t count = 0; // the line count
    size_t output_idx = 0;

    while (stop == 0) {
        pthread_mutex_lock(&mutex1);
        while (! (count < out_idx1)) { // no new info in buffer
            pthread_cond_wait(&buf1_full, &mutex1);
        }
        pthread_mutex_unlock(&mutex1);

        if (strcmp(buffer1[count], str) == 0) {
            stop = 1;
            strcpy(buffer2[count], str);
        } // Normal exit

        else {
            ssize_t len = sizeof(buffer1[count]);
            output_idx = 0;
            for (size_t n = 0; n < len; ++n) {
                buffer2[count][output_idx] = (buffer1[count][n] == '\n') ? ' ' :
                                            buffer1[count][n];
                output_idx++;
            }
        }

        count += 1;

        if (pthread_mutex_trylock(&mutex2) == 0 || stop == 1) { // need to update if stopping
            out_idx2 = count;
            pthread_mutex_unlock(&mutex2);
            pthread_cond_signal(&buf2_full);
        }

    }
    pthread_cond_signal(&buf2_full);

    return NULL;
}

void *plusSign(void *args){
    int stop = 0;
    size_t count = 0; // the line count
    size_t output_idx = 0;

    while (stop == 0) {
        pthread_mutex_lock(&mutex2);
        while (! (count < out_idx2)) { // no new info in buffer
            pthread_cond_wait(&buf2_full, &mutex2);
        }
        pthread_mutex_unlock(&mutex2);

        if (strcmp(buffer2[count], str) == 0) {
            stop = 1;
            strcpy(buffer3[count], str);
        } // Normal exit

        else {
            output_idx = 0;
            ssize_t len = sizeof(buffer2[count]);
            for (size_t n = 0; n < len; ++n) {
                buffer3[count][output_idx] = (buffer2[count][n] == '+' && buffer2[count][n+1] == '+') ? n+=1, '^' :
                                             buffer2[count][n];
                output_idx++;
            }
        }
        count += 1;

        if (pthread_mutex_trylock(&mutex3) == 0 || stop == 1) { // need to update if stopping
            out_idx3 = count;
            pthread_mutex_unlock(&mutex3);
            pthread_cond_signal(&buf3_full);
        }

    }
    pthread_cond_signal(&buf3_full);

    return NULL;
}

void *output(void *args){
    int stop = 0;
    size_t count = 0; // the line count
    size_t output_idx = 0; // count in intervals of 80
    char buffer4[80]; // buffer to hold the output

    while (stop == 0) {
        pthread_mutex_lock(&mutex3);
        while (! (count < out_idx3)) { // no new info in buffer
            pthread_cond_wait(&buf3_full, &mutex3);
        }
        pthread_mutex_unlock(&mutex3);

        if (strcmp(buffer2[count], str) == 0) {
            stop = 1;
        } // Normal exit

        else {
            size_t len = strlen(buffer3[count]);
            for (size_t n = 0; n < len; ++n) {
                buffer4[output_idx] = buffer3[count][n];
                if (++output_idx == 80) {
                    fwrite(buffer4, 1, 80, stdout);
                    putchar('\n');
                    fflush(stdout);
                    output_idx = 0;
                    memset(buffer4, 0, sizeof buffer4);
                }
            }
        }

        count += 1;

    }

    return NULL;
}

int main(void) {
    buffer1 = malloc(sizeof(int[LINE][SIZE]));
    buffer2 = malloc(sizeof(int[LINE][SIZE]));
    buffer3 = malloc(sizeof(int[LINE][SIZE]));

    pthread_t in, ls, ps, op;
    pthread_create(&in, NULL, getInput, NULL);
    pthread_create(&ls, NULL, lineSeparator, NULL);
    pthread_create(&ps, NULL, plusSign, NULL);
    pthread_create(&op, NULL, output, NULL);
    pthread_join(in, NULL);
    pthread_join(ls, NULL);
    // fprintf(stderr, "ls finished");
    pthread_join(ps, NULL);
    // fprintf(stderr, "ps finished");
    pthread_join(op, NULL);
    //fprintf(stderr, "op finished");
    free(buffer1);
    free(buffer2);
    free(buffer3);
    return 0;
}