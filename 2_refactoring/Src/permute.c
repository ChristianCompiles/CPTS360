#include <stdio.h>
#include <stdlib.h>
#include "gen_perms.h"

/* printPermutation -- print a permutation of an array of char *'s */
static void printPermutation(int indices[], int nIndices, void *userArg)
{
    int i;
    char **syms = userArg;

    for (i = 0; i < nIndices; i++)
        printf("%s ", syms[indices[i]]);
    printf("\n");
}

int main(int argc, char *argv[])
{
    // printf("FROM MAIN:\n");
    // for (int i = 1; i < argc; i++)
    // {
    //     printf("%s\n", argv[i]);
    // }
    //printf("\n");

    genPerms(argc-1, &printPermutation, &argv[1]);
    return 0;
}
