#include <mpich/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *NextWordPtr(char *sentense, int *trimmed_word_length) {
    char *p = sentense;
    *trimmed_word_length = 0;
    while (*p != '\0' && *p != ' ') {
        p++;
        (*trimmed_word_length)++;
    }
    if (*p == '\0') {
        return NULL;
    }

    while (*p != '\0' && *p == ' ') {
        p++;
    }
    if (*p == '\0') {
        return NULL;
    }
    return p;
}

int main() {
    char a[] = "aaaa bbbb cccc dddd eeeee ffff ggg hhhh iiii jjjj kkk llll\0";
    char b[70];

    int trim = 0;
    char *p = a;
    char cpy[10];
    while (1) {
        char *tmp = p;
        p = NextWordPtr(p, &trim);
        if (p == NULL) {
            return 0;
        }
        strncpy(cpy, tmp, trim);
        cpy[trim] = '\0';
        printf("cpy: %s, p: %s, trim: %d\n", cpy, p, trim);

        strcpy(b, p);
        printf("b: %s\n", b);
    }
}
