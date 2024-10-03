// nElems: is the length of the permutation

// handlePerm(): is a "callback": it is called once for each permutation with these arguments:

    // elems: is an array of permuted indices.

    // nElems: is the length of the permutation (and is equal to the nElems argument of permute())

    // userArg: is identical to the userArg passed to permute() (see below)

// userArg: is an arbitrary pointer passed on by permute(). 
//          This is a common technique in C system programming. 
//          Take a look at permute.c and you'll see how using it avoids the need for global variables. 
//          (This means there is no problem using genPerms() with threads.)
#include <stdio.h>
#include <stdlib.h>
enum {
    N_ELEM = 3,
    NOT_DONE = -1
};

void recur(int level, int* val, int k, int nElems, void* userArg, void (*handlePerm)(int elems[], int nElems, void *userArg))
{
    int i;
    val[k] = level;
    level++;
    //printf("value of level: %d\n", level);
    if (level == nElems) {
        //printf("About to print in recur\n");
        handlePerm(val, nElems, userArg);
        printf("\n");
    }
    for (i = 0; i < nElems; i++)
        if (val[i] == NOT_DONE)
            recur(level, val, i, nElems, userArg, handlePerm);
    level--;
    val[k] = NOT_DONE;
}

void genPerms(int nElems, 
         void (*handlePerm)(int elems[], int nElems, void *userArg), 
         void *userArg)
{

    // beginning of testing values // 
    // char ** userInput = userArg; // cast

    // printf("FROM genPerms:\n");
    // for (int i = 0; i < nElems; i++)
    // {
    //     printf("%d , %s\n", i, userInput[i]);
    // }
    // printf("\n");
    // end of testing values // 

    int* val = (int*)malloc(nElems * sizeof(int)); // dynamically size val

    for (int i = 0; i < nElems; i++) // init values to -1
        val[i] = NOT_DONE;

    // for (int i = 0; i < nElems; i++) // print values of val (should be -1)
    // {
    //     printf("%d\n", val[i]);
    // }

    int level = 0;
    for (int i = 0; i < nElems; i++)
        recur(level, val, i, nElems, userArg, handlePerm);
}
