#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

int main(void)
{
    char *line = NULL;
    size_t n = 0;
    char outbuf1[80];
    size_t op = 0;

    /* TODO : Input thread */

    for (;;) {

        ssize_t len = getline(&line, &n, stdin);
        if (len == -1) {
            if (feof(stdin)) break; // EOF is not defined by the spec. I treat it as "STOP\n"
            else err(1, "stdin");
        }
        if (strcmp(line, "STOP\n") == 0) break; // Stop processing line
        for (size_t n = 0; n < len; ++n) {
            outbuf1[op] = (line[n] == '+' && line[n+1] == '+') ? n+=1, '^' :
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