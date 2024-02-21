#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

char outbuf1[80];
char outbuf2[80];
char outbuf3[80];

void *getInput(void *args){
    char *line = NULL;
    size_t n = 0;
    size_t op = 0;

    for (;;) {
        ssize_t len = getline(&line, &n, stdin);
        if (len == -1) {
            if (feof(stdin)) break; // EOF is not defined by the spec. I treat it as "STOP\n"
            else err(1, "stdin");
        }
        if (strcmp(line, "STOP\n") == 0) break; // Normal exit
        for (size_t n = 0; n < len; ++n) {
            outbuf1[op] = line[n];
            if (++op == 80) {
                fwrite(outbuf1, 1, 80, stdout);
                putchar('\n');
                fflush(stdout);
                op = 0;
            }
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