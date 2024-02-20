#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

void *getInput(void *args){
    printf("Hello World!\n");
    return NULL;
}

void *lineSeparator(void *args){
    printf("Hello World!\n");
    return NULL;
}

void *plusSign(void *args){
    printf("Hello World!\n");
    return NULL;
}

void *Output(void *args){
    printf("Hello World!\n");
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