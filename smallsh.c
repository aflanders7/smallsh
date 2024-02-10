#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#ifndef MAX_WORDS
#define MAX_WORDS 512
#endif

#define O_WRONLY         01
#define O_CREAT          0100
#define O_TRUNC          01000
#define O_APPEND          02000

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
struct sigaction sigint_action = {0}, ignore_action = {0}, sigint_old = {0}, sigstp_old = {0};
void sigint_handler(int sig) {}

char *words[MAX_WORDS];
size_t wordsplit(char const *line);
char * expand(char const *word);
int foreground = 0;
int background = 0;
int bgpid = 0;
pid_t childPID;

char *childwords[MAX_WORDS] = {0};
int childStatus = 0;
int spawnStatus = 0;

int main(int argc, char *argv[])
{
    FILE *input = stdin;
    char *input_fn = "(stdin)";
    if (argc == 2) {
        input_fn = argv[1];
        input = fopen(input_fn, "re");
        if (!input) err(1, "%s test1", input_fn);
    } else if (argc > 2) {
        errx(1, "too many arguments");
    }

    char *line = NULL;
    size_t n = 0;
    char exit_str[] = "exit";
    char cd_str[] = "cd";
    char line_str[] = " ";
    for (;;) {
prompt:;
        /* TODO: Manage background processes */

        while (waitpid(0, &spawnStatus, WNOHANG | WUNTRACED) > 0) {
            if (WIFSTOPPED(spawnStatus)){
                kill(childPID, SIGCONT);
                fprintf(stderr, "Child process %d stopped. Continuing.\n", childPID);
            }
            if (WIFSIGNALED(spawnStatus)){
                fprintf(stderr, "Child process %d done. Signaled %d.\n", childPID, WTERMSIG(spawnStatus));
            }
            if (WIFEXITED(spawnStatus)){
                fprintf(stderr, "Child process %d done. Exit status %d.\n", childPID, WEXITSTATUS(spawnStatus));
            }
        }


        /* TODO: prompt ; interactive otherwise it's a file*/
        if (input == stdin) {
            sigint_action.sa_handler = sigint_handler;
            sigaction(SIGINT, &sigint_action, &sigint_old);

            ignore_action.sa_handler = SIG_IGN;
            sigaction(SIGTSTP, &ignore_action, &sigstp_old);

        }

        ssize_t line_len = getline(&line, &n, input);
        if (line_len < 0) { exit(0); }

        if (*line == '\n') {
            goto prompt;
            /* TODO segmentation fault with just a space */
        }

        size_t nwords = wordsplit(line);

        for (size_t i = 0; i < nwords; ++i) {
            /* fprintf(stderr, "Word %zu: %s\n", i, words[i]); */
            char *exp_word = expand(words[i]);
            free(words[i]);
            words[i] = exp_word; /* This is needed to print to stdout */
            /* fprintf(stdout, "%s", words[i]); */
        }

        if (strcmp(words[0], exit_str) == 0) {
            if (nwords > 2) {
                fprintf(stderr, "Too many arguments given.");
            }
            /* TODO: add in foreground exit status */
            if (nwords == 1) {exit(childStatus);}
            char *endptr;
            long int digit = strtol(words[1], &endptr, 10);
            if (*endptr != '\0') {
                fprintf(stderr, "Argument is not an integer.");
            }
            else {
                int val = atoi(words[1]);
                exit(val);
            }
        }

        else if (strcmp(words[0], cd_str) == 0) {
            if (nwords > 2) {
                fprintf(stderr, "Too many arguments given.");
            }
            if (nwords == 1) {chdir(getenv("HOME"));
                /* char buffer[FILENAME_MAX];
                * getcwd(buffer, FILENAME_MAX );
                * printf(buffer); */
            }
            else {
                if (chdir(words[1]) != 0) {
                    fprintf(stderr, "Changing directory failed.");
                }
                /* char buffer[FILENAME_MAX];
                * getcwd(buffer, FILENAME_MAX );
                * printf(buffer); */
            }
        }
        else {
            pid_t spawnPID = fork();

            switch (spawnPID) {
                case -1:
                    perror("fork() failed\n");
                    exit(1);
                    break;
                case 0:
                    if (input == stdin) {
                        sigaction(SIGTSTP, &sigstp_old, NULL);
                        sigaction(SIGINT, &sigint_old, NULL);
                    }

                    fflush(stdout);
                    for (size_t i = 0; i < nwords; ++i) {
                        if (i == nwords - 1 && strcmp(words[i], "&") == 0) {
                            /* skips adding & */
                        }
                        else if (strcmp(words[i], "<") == 0) {
                            if (i + 1 == nwords) {
                                fprintf(stderr, "No file specified.");
                            } else {
                                freopen(words[i + 1], "r", stdin);
                                /*TODO : Handle error, close file */
                                i = i + 1;
                            }
                        } else if (strcmp(words[i], ">") == 0) {
                            if (i + 1 == nwords) {
                                fprintf(stderr, "No file specified.");
                            } else {
                                int file = open(words[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0777);
                                if (file == -1) {
                                    perror("open()");
                                    exit(1);
                                }
                                int result = dup2(file, 1);
                                if (result == -1) {
                                    perror("dup2");
                                    exit(2);
                                }
                                i = i + 1;
                            }
                        } else if (strcmp(words[i], ">>") == 0) {
                            if (i + 1 == nwords) {
                                fprintf(stderr, "No file specified.");
                            } else {
                                int file1 = open(words[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0777);
                                if (file1 == -1) {
                                    perror("open()");
                                    exit(1);
                                }
                                int result = dup2(file1, 1);
                                if (result == -1) {
                                    perror("dup2");
                                    exit(2);
                                }
                                i = i + 1;
                            }
                        } else {
                            childwords[i] = words[i];
                        }
                    }

                    if (execvp(childwords[0], childwords) == -1) {
                        fprintf(stderr, "failed to exec");
                    }

                    exit(childStatus);

                    /*TODO : */

                default:
                    if (strcmp(words[nwords - 1], "&") == 0) {
                        /* tested, works */
                        background = 1;
                        bgpid = spawnPID;
                        childPID = spawnPID;
                    }
                    else {
                        background = 0;
                    }
                    if (background == 0) {
                        spawnPID = waitpid(spawnPID, &childStatus, WUNTRACED);
                        if (WIFSTOPPED(childStatus)){
                            kill(spawnPID, SIGCONT);
                            fprintf(stderr, "Child process %d stopped. Continuing.\n", spawnPID);
                            bgpid = spawnPID;
                        }
                        else if (WIFSIGNALED(childStatus) && background == 0){
                            foreground = 128 + WTERMSIG(childStatus);
                        }
                        else {foreground = WEXITSTATUS(childStatus);}
                    }

                    else {
                        /* TODO SOMETHING */
                        goto prompt;
                    }
                    break;
            }
            /* TODO: reset signals;*/
        }
    }
    return 0;
}

char *words[MAX_WORDS] = {0};


/* Splits a string into words delimited by whitespace. Recognizes
 * comments as '#' at the beginning of a word, and backslash escapes.
 *
 * Returns number of words parsed, and updates the words[] array
 * with pointers to the words, each as an allocated string.
 */
size_t wordsplit(char const *line) {
    size_t wlen = 0;
    size_t wind = 0;

    char const *c = line;
    for (;*c && isspace(*c); ++c); /* discard leading space */

    for (; *c;) {
        if (wind == MAX_WORDS) break;
        /* read a word */
        if (*c == '#') break;
        for (;*c && !isspace(*c); ++c) {
            if (*c == '\\') ++c;
            void *tmp = realloc(words[wind], sizeof **words * (wlen + 2));
            if (!tmp) err(1, "realloc");
            words[wind] = tmp;
            words[wind][wlen++] = *c;
            words[wind][wlen] = '\0';
        }
        ++wind;
        wlen = 0;
        for (;*c && isspace(*c); ++c);
    }
    return wind;
}


/* Find next instance of a parameter within a word. Sets
 * start and end pointers to the start and end of the parameter
 * token.
 */
char
param_scan(char const *word, char const **start, char const **end)
{
    static char const *prev;
    if (!word) word = prev;

    char ret = 0;
    *start = 0;
    *end = 0;
    for (char const *s = word; *s && !ret; ++s) {
        s = strchr(s, '$');
        if (!s) break;
        switch (s[1]) {
            case '$':
            case '!':
            case '?':
                ret = s[1];
                *start = s;
                *end = s + 2;
                break;
            case '{':;
                char *e = strchr(s + 2, '}');
                if (e) {
                    ret = s[1];
                    *start = s;
                    *end = e + 1;
                }
                break;
        }
    }
    prev = *end;
    return ret;
}

/* Simple string-builder function. Builds up a base
 * string by appending supplied strings/character ranges
 * to it.
 */
char *
build_str(char const *start, char const *end)
{
    static size_t base_len = 0;
    static char *base = 0;

    if (!start) {
        /* Reset; new base string, return old one */
        char *ret = base;
        base = NULL;
        base_len = 0;
        return ret;
    }
    /* Append [start, end) to base string
     * If end is NULL, append whole start string to base string.
     * Returns a newly allocated string that the caller must free.
     */
    size_t n = end ? end - start : strlen(start);
    size_t newsize = sizeof *base *(base_len + n + 1);
    void *tmp = realloc(base, newsize);
    if (!tmp) err(1, "realloc");
    base = tmp;
    memcpy(base + base_len, start, n);
    base_len += n;
    base[base_len] = '\0';

    return base;
}

/* Expands all instances of $! $$ $? and ${param} in a string 
 * Returns a newly allocated string that the caller must free
 */
char *
expand(char const *word)
{
    char const *pos = word;
    char const *start, *end;
    char c = param_scan(pos, &start, &end);
    build_str(NULL, NULL);
    build_str(pos, start);
    while (c) {
        if (c == '!') {
            if (background == 0 && bgpid == 0){
                build_str("", NULL);
            }
            else {
                char *pid;
                int get_pid = asprintf(&pid, "%d", bgpid);
                build_str(pid, NULL);
                free(pid);
            }
        }
        else if (c == '$') {
            char *pid;
            int get_pid = asprintf(&pid, "%d", getpid());
            build_str(pid, NULL);
            free (pid);
        }
        else if (c == '?') {
            char *pid;
            int get_pid = asprintf(&pid, "%d", foreground);
            build_str(pid, NULL);
            free(pid);
        }
        else if (c == '{') {
            char const *param = word;
            char *parameter;
            parameter = (char *) malloc(MAX_WORDS);
            strncpy(parameter, start+2, strlen(param)-3);
            if (getenv(parameter) == NULL) {build_str("", NULL);}
            else {build_str(getenv(parameter), NULL);}
            free(parameter);
        }
        pos = end;
        c = param_scan(pos, &start, &end);
        build_str(pos, start);
    }
    return build_str(start, NULL);
}
