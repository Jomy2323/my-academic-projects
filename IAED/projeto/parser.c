#include "commands.h"
#include "parser.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int readLine(char* s){
    int c, i = 0;
    while ((c = getchar()) != '\n' && c != EOF && i < BUFSIZ - 1){
        s[i++] = c;
    }
    s[i] = '\0';
    return i;
}

char* getName(char* s, int* currentIndex){
    int mallocSize = 0;
    int cursor = (s[0] == '"') ? 1 : 0;
    char stopChar = (s[0] == '"') ? '"' : ' ';
    while (s[cursor] != stopChar && s[cursor] != '\0'){
        mallocSize++;
        cursor++;
    }

    char* name = (char*)malloc((mallocSize + 1)*sizeof(char));
    if (name == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strncpy(name, (s[0] == '"') ? s+1 : s, mallocSize);
    name[mallocSize] = '\0';    
    *currentIndex += cursor + (s[0] == '"' ? 1 : 0);
    return name;
}