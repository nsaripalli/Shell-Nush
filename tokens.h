

#ifndef TOKENS_H
#define TOKENS_H

#include "svec.h"

int tok_line(const char *text, int ii);
int tokenize(const char *text);
int read_line();
char * read_line_from_file(int file);
svec *getInput();
svec *getFileInput(int file);

#endif