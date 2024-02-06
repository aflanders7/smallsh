#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#ifndef MAX_WORDS
#define MAX_WORDS 512
#endif

char *words[MAX_WORDS];
size_t wordsplit(char const *line);
char * expand(char const *word);
int foreground = 0;
char const *background = "";


int main(int argc, char *argv[])
{
  FILE *input = stdin;
  char *input_fn = "(stdin)";
  if (argc == 2) {
    input_fn = argv[1];
    input = fopen(input_fn, "re");
    if (!input) err(1, "%s", input_fn);
  } else if (argc > 2) {
    errx(1, "too many arguments");
  }

  char *line = NULL;
  size_t n = 0;
  char exit_str[] = "exit";
  char cd_str[] = "cd";
  for (;;) {
//prompt:;
    /* TODO: Manage background processes */

    /* TODO: prompt ; interactive otherwise it's a file*/
    if (input == stdin) {

    }
    ssize_t line_len = getline(&line, &n, input);
    if (line_len < 0) err(1, "%s", input_fn);
    
    size_t nwords = wordsplit(line);

    if (strcmp(words[0], exit_str) == 0) {
        if (nwords > 2) {
            fprintf(stderr, "Too many arguments given.");
        }
        /* TODO: add in foreground exit status */
        if (nwords == 1) {exit(foreground);}
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
        int childStatus;
        pid_t childPid = fork();

        if(childPid == -1){
            perror("fork() failed.");
            exit(1);
        }
        else if(childPid == 0){
            if (execvp(words[0], words) == -1) {fprintf(stderr, "Child failed to exec.");}

        }
        else {
            childPid = waitpid(childPid, &childStatus, 0);
            if(WIFEXITED(childStatus)){
                printf("Child %d exited normally with status %d\n", childPid, WEXITSTATUS(childStatus));
            } else{
                fprintf(stderr, "Child %d exited abnormally due to signal %d\n", childPid, WTERMSIG(childStatus));
                exit(1);
            }
        }
        return 0;
        /* TODO: reset signals, redirection */
    }

    for (size_t i = 0; i < nwords; ++i) {
      /* fprintf(stderr, "Word %zu: %s\n", i, words[i]); */
      char *exp_word = expand(words[i]);
      free(words[i]);
      words[i] = exp_word;
      /* fprintf(stderr, "Expanded Word %zu: %s\n", i, words[i]); */
    }
  }
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
    if (c == '!') build_str("<BGPID>", NULL);
    else if (c == '$') {
        char *pid;
        /* int get_pid = asprintf(&pid, "%d", getpid()); */
        build_str(pid, NULL);
        free (pid);}
    else if (c == '?') build_str("<STATUS>", NULL);
    else if (c == '{') {
      char const *param = word;
      char *parameter;
      strncpy(parameter, start+2, strlen(param)-3);
      if (getenv(parameter) == NULL) {build_str("", NULL);}
      else {build_str(getenv(parameter), NULL);}
    }
    pos = end;
    c = param_scan(pos, &start, &end);
    build_str(pos, start);
  }
  return build_str(start, NULL);
}
