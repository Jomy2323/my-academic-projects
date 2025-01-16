#ifndef PARSER_H
#define PARSER_H

#include "common.h"

int readLine(char* s);
int readArgumentSize(char* s);
char* getName(char* s, int* currentIndex);

#endif /* PARSER_H */
