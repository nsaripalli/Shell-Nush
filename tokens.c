#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "svec.h"

//TODO: Use REGEX to parse the input

//Parts from lecture 09-calculator/calc/tokens.c
char *
tok_line(const char *text, int ii) {
    int nn = 0;
    while (isgraph(text[ii + nn])) {
        if (text[ii + nn] == ';'
            || text[ii + nn] == '|'
            || ((text[ii + nn] == '&') && (text[ii + nn + 1] == '&'))
                ) {
            break;
        }
        ++nn;
    }

    char *num = malloc(nn + 1);
    memcpy(num, text + ii, nn);
    num[nn] = 0;
    return num;
}

svec *
tokenize(const char *text) {
    svec *xs = make_svec();

    int nn = (int) strlen(text);
    int ii = 0;
    char *ampamp = "&&";
    while (ii < nn) {
        if (isspace(text[ii])) {
            ++ii;
            continue;
        } else if (text[ii] == ';') {
            char semic = ';';
            char *psemi = malloc(2 * sizeof(char));
            psemi[0] = semic;
            psemi[1] = 0;
            svec_push_back(xs, psemi);
            ii += strlen(psemi);
            free(psemi);
            continue;
        } else if (text[ii] == '|' && text[ii + 1] == '|') {
            char *psemi = malloc(3 * sizeof(char));
            psemi[0] = '|';
            psemi[1] = '|';
            psemi[2] = 0;
            svec_push_back(xs, psemi);
            ii += strlen(psemi);
            free(psemi);
            continue;
        } else if (text[ii] == '|') {
            char semic = '|';
            char *psemi = malloc(2 * sizeof(char));
            psemi[0] = semic;
            psemi[1] = 0;
            svec_push_back(xs, psemi);
            ii += strlen(psemi);
            free(psemi);
            continue;
        } else if ((text[ii] == '&') && (text[ii + 1] == '&')) {
            char *semic = "&&";
            char *psemi = malloc((strlen(semic) + 1) * sizeof(char));
            psemi[0] = '&';
            psemi[1] = '&';
            psemi[2] = 0;
            svec_push_back(xs, psemi);
            ii += strlen(psemi);
            free(psemi);
            continue;
        } else if (isgraph(text[ii])) {
            char *num = tok_line(text, ii);
            svec_push_back(xs, num);
            ii += strlen(num);
            free(num);
            continue;
        }

        ++ii;
    }

    return xs;
}


//This is mostly from lecture 07-asm-syscalls with slight modifications
//I am using this instead of fgets so I will never run out of buffer no matter
//how long the line is
char *
read_line() {
    int cap = 50 * (sizeof(char));
    char *buf = malloc(cap);
    long ii = 0;

    for (;;) {
        if (ii >= cap) {
            cap *= 2;
            buf = realloc(buf, cap);
        }

        long count = read(0, buf + ii, 1);
        if (buf[ii] == '\n') {
            break;
        }

        if (count == 0) {
            exit(0);
        }
        ++ii;
    }
    buf[ii] = 0;

    char *yy = strdup(buf);
    free(buf);
    return yy;
}


//This is mostly from lecture 07-asm-syscalls with slight modifications
//I am using this instead of fgets so I will never run out of buffer no matter
//how long the line is
char *
read_line_from_file(int file) {
    int cap = 50 * (sizeof(char));
    char *buf = malloc(cap);
    long ii = 0;

    for (;;) {
        if (ii >= cap) {
            cap *= 2;
            buf = realloc(buf, cap);
        }

        long count = read(file, buf + ii, 1);
        if (buf[ii] == '\n') {
            break;
        }

        if (count == 0) {
            exit(0);
        }
        ++ii;
    }
    buf[ii] = 0;

    char *yy = strdup(buf);
    free(buf);
    return yy;
}

svec *getInput() {
    char *line;
    int isSucessful;
    svec *tokens;
    int nn;

    fflush(stdout);
    line = read_line();
    if (line == 0) {
        return 0;
    }
    tokens = tokenize(line);
    free(line);

    return tokens;
}


svec *getFileInput(int file) {
    char *line;
    int isSucessful;
    svec *tokens;
    int nn;

    fflush(stdout);
    line = read_line_from_file(file);
    if (line == 0) {
        return 0;
    }
    tokens = tokenize(line);
    free(line);

    return tokens;
}