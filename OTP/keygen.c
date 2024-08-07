#include <stdio.h>
#include <stdlib.h>

static char const allowed_characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

int main (int argc, char *argv[])
{
    int count;
    count = atoi(argv[1]);
    for (int i = 0; i < count; ++i) {
        int index = rand() % 27;
        char c = allowed_characters[index];
        putchar(c);
    }
    putchar('\n');
    fflush(stdout);
    return 0;
}