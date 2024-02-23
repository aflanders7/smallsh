#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 1000

// buffer for input and line separator
char outbuf1[SIZE];
// index input
int out_shared1 = 0; // using out shared index to keep track of if the buffer is full
// mutex
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
// conditionals
pthread_cond_t out_cond1 = PTHREAD_COND_INITIALIZER;


// buffer for line separator and plus sign
char outbuf2[SIZE];
// index input
int out_shared2 = 0;
// mutex
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
// conditionals
pthread_cond_t out_cond2 = PTHREAD_COND_INITIALIZER;


// buffer for plus sign and output
char outbuf3[SIZE];
// index input
int out_shared3 = 0;
// mutex
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
// conditionals
pthread_cond_t out_cond3 = PTHREAD_COND_INITIALIZER;


void *getInput(void *args){
    char *line = NULL;
    size_t n = 0;
    char outbuf[80];
    size_t op = 0;
    for (;;) {
        ssize_t len = getline(&line, &n, stdin);
        if (len == -1) {
            if (feof(stdin)) break; // EOF is not defined by the spec. I treat it as "STOP\n"
            else err(1, "stdin");
        }
        if (strcmp(line, "STOP\n") == 0) break; // Normal exit
        for (size_t n = 0; n < len; ++n) {
            outbuf[op] = (line[n] == '+' && line[n+1] == '+') ? n+=1, '^' :
                         (line[n] == '\n')                    ? ' ' :
                         line[n];
            if (++op == 80) {
                fwrite(outbuf, 1, 80, stdout);
                putchar('\n');
                fflush(stdout);
                op = 0;
            }
        }
    }
    free(line);
}

void *lineSeparator(void *args){
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