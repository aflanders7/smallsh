#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>


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
int out_shared2 = 0;
// mutex
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
// conditionals
pthread_cond_t out_cond2 = PTHREAD_COND_INITIALIZER;


// buffer for plus sign and output
char outbuf3[80];
// index input
int out_shared3 = 0;
// mutex
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
// conditionals
pthread_cond_t out_cond3 = PTHREAD_COND_INITIALIZER;


void *getInput(void *args){
    char *line = NULL;

    size_t in_current_idx = 0;
    size_t out_current_idx = 0;

    for (;;) {
        while (out_current_idx < 80) {
            ssize_t len = getline(&line, &in_current_idx, stdin); // read data
            if (len == -1) {
                if (feof(stdin)) return NULL; // EOF treated as "STOP\n"
                else err(1, "stdin");
            }
            if (strcmp(line, "STOP\n") == 0) return NULL; // TODO Normal exit; do I need to return to exit?

            char c = line[in_current_idx++]; // increments after it's used

            // write to the shared buffer; NOTE: out current index is local
            outbuf1[out_current_idx] = c;

            // no in shared index, so skip for this thread
            // write to shared output index and signal that more is available
            // signals that more has been written
            if (pthread_mutex_trylock(&mutex1) == 0) {
                out_shared1 = out_current_idx;
                pthread_mutex_unlock(&mutex1);
                pthread_cond_signal(&out_cond1);
            }
            out_current_idx++;
        }

        // check if buffer is full;  this could be an issue and block the mutex! might need to try lock
        // TODO consider making this an if loop so that mutex will not be blocked

        if (out_current_idx == 80) {
            pthread_mutex_lock(&mutex1);
            out_shared1 = out_current_idx;
            while (out_shared1 == 80) {
                pthread_mutex_unlock(&mutex1);
                sleep(1);
                pthread_mutex_lock(&mutex1);
            }
            pthread_mutex_unlock(&mutex1);
        }
    }
    free(line);
}

void *lineSeparator(void *args){

    size_t in_current_idx = 0;
    size_t in_max_idx = 0;
    size_t out_current_idx = 0;

    for (;;) {
        // in_max_idx max val is 80, if in_current = 80 it will stop
        while (in_current_idx < in_max_idx) {
            // Read data from shared buffer
            char c = outbuf1[in_current_idx++];

            // process data...
            // write data to shared buffer
            outbuf2[out_current_idx] = (c == '\n') ? ' ' :
                                         c;

            // Attempt to read the shared index, but don't block
            // if the mutex is currently locked.
            if (pthread_mutex_trylock(&mutex1) == 0) {
                in_max_idx = out_shared1;
                pthread_mutex_unlock(&mutex1);
            }
            // Attempt to write the shared index, but don't block
            // if the mutex is currently locked.
            if (pthread_mutex_trylock(&mutex2) == 0) {
                out_shared2 = out_current_idx;
                pthread_mutex_unlock(&mutex2);
                pthread_cond_signal(&out_cond2);
            }

            out_current_idx++;
            if (out_current_idx == 80) break;
        }
        // We cannot continue without updating in_max_idx
        // First update shared out index for downstream thread

        pthread_mutex_lock(&mutex2);
        out_shared2 = out_current_idx;
        // check for buffer to be cleared; do I need this or can you re-write index to loop without clearing
        if (out_current_idx == 80){
            while (out_shared2 == 80) {
                pthread_mutex_unlock(&mutex2);
                sleep(1);
                pthread_mutex_lock(&mutex2);
            }
        }
        else {
            out_shared2 = out_current_idx;
            pthread_cond_signal(&out_cond2);
        }
        pthread_mutex_unlock(&mutex2);

        // Now wait for upstream thread to put data in for us
        pthread_mutex_lock(&mutex1);
        in_max_idx = out_shared1;
        // buffer is full, overwrite the index
        if (in_current_idx == 80) {
            out_shared1 = 0;
            in_current_idx = 0;
            in_max_idx = 0;
        }
        while (! (in_current_idx < in_max_idx)) {
            pthread_cond_wait(&out_cond1, &mutex1);
            in_max_idx = out_shared1;
        }
        pthread_mutex_unlock(&mutex1);
    }
}

void *plusSign(void *args){

    size_t in_current_idx = 0;
    size_t in_max_idx = 0;
    size_t out_current_idx = 0;
    for (;;) {
        while (in_current_idx < in_max_idx) {
            // Read data from shared buffer
            char c = outbuf2[in_current_idx];
            char d = outbuf2[in_current_idx+1];
            in_current_idx++;

            // process data...
            // write data to shared buffer
            outbuf3[out_current_idx] = (c == '+' && d == '+') ? in_current_idx+=1, '^' :
                                         c;

            // Attempt to read the shared index, but don't block
            // if the mutex is currently locked.
            if (pthread_mutex_trylock(&mutex2) == 0) {
                in_max_idx = out_shared2;
                pthread_mutex_unlock(&mutex2);
            }
            // Attempt to write the shared index, but don't block
            // if the mutex is currently locked.
            if (pthread_mutex_trylock(&mutex3) == 0) {
                out_shared3 = out_current_idx;
                pthread_mutex_unlock(&mutex3);
                pthread_cond_signal(&out_cond3);
            }

            out_current_idx++;
            if (out_current_idx == 80) break;

        }

        pthread_mutex_lock(&mutex3);
        out_shared3 = out_current_idx;
        // check for buffer to be cleared; do I need this or can you re-write index to loop without clearing
        if (out_current_idx == 80){
            while (out_shared3 == 80) {
                pthread_mutex_unlock(&mutex3);
                sleep(1);
                pthread_mutex_lock(&mutex3);
            }
        }
        else {
            out_shared3 = out_current_idx;
            pthread_cond_signal(&out_cond3);
        }
        pthread_mutex_unlock(&mutex3);

        // Now wait for upstream thread to put data in for us
        pthread_mutex_lock(&mutex2);
        in_max_idx = out_shared2;
        // buffer is full, overwrite the index
        if (in_current_idx == 80) {
            out_shared2 = 0;
            in_current_idx = 0;
            in_max_idx = 0;
        }
        while (! (in_current_idx < in_max_idx)) {
            pthread_cond_wait(&out_cond2, &mutex2);
            in_max_idx = out_shared1;
        }
        pthread_mutex_unlock(&mutex2);
    }
}

void *Output(void *args){

    size_t in_max_idx = 0;

    for (;;) {
        if (++in_max_idx == 80) {
            // Read data from shared buffer
            fwrite(outbuf3, 1, 80, stdout);
            putchar('\n');
            fflush(stdout);
            pthread_mutex_lock(&mutex3);
            out_shared3 = 0;    // reset the index
            in_max_idx = out_shared3;
            pthread_mutex_unlock(&mutex3);
        }

        // Waiting for the upstream (input) to put the data in
        pthread_mutex_lock(&mutex3);
        in_max_idx = out_shared3;
        while (++in_max_idx != 80) {
            pthread_cond_wait(&out_cond3, &mutex3);
            in_max_idx = out_shared3;
        }
        pthread_mutex_unlock(&mutex3);
    }
}

int main(void) {
    pthread_t in, ls, ps, op;
    pthread_create(&in, NULL, getInput, NULL);
    pthread_create(&ls, NULL, lineSeparator, NULL);
    pthread_create(&ps, NULL, plusSign, NULL);
    pthread_create(&op, NULL, Output, NULL);
    pthread_join(in, NULL);
    pthread_join(ls, NULL);
    pthread_join(ps, NULL);
    pthread_join(op, NULL);
}