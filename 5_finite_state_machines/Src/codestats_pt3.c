#include <stdio.h>
#include <assert.h> // for assert()
#include <ctype.h> // for isspace()

struct CodeStats {
    int lineCount;
    int linesWithCodeCount;
    int cplusplusCommentCount;
};


void codeStats_init(struct CodeStats *codeStats)
{
    codeStats->lineCount = 0;
    codeStats->linesWithCodeCount = 0;
    codeStats->cplusplusCommentCount = 0;
}


void codeStats_print(struct CodeStats codeStats, char *fileName)
{
    printf("     file: %s\n", fileName);
    printf("     line count: %d\n", codeStats.lineCount);
    printf("     lines with code count: %d\n", codeStats.linesWithCodeCount);
    printf("     cpluspluscommentcount: %d\n", codeStats.cplusplusCommentCount);
}


void codeStats_accumulate(struct CodeStats *codeStats, char *fileName)
{
    FILE *f = fopen(fileName, "r");
    int ch;
    enum {
        START,
        FOUNDSLASH,
        INCPPCOMMENT,
    } state = START;

    int foundCodeOnLine = 0;

    assert(f);
    while ((ch = getc(f)) != EOF) {
        switch (state) {

            case START: // start state
                if (ch == '\n') {
                    codeStats->lineCount++;
                    codeStats->linesWithCodeCount += foundCodeOnLine;
                    foundCodeOnLine = 0;
                    continue;
                }

                if (isspace(ch)) // do nothing on space char
                    continue;

                if (ch == '/') {
                    state = FOUNDSLASH; // go to FOUND state
                }
                else // there must be a char
                    foundCodeOnLine = 1;
                break; // break after case START

            case FOUNDSLASH: // FOUND STATE
                if(ch == '\n')
                {
                    foundCodeOnLine = 1;
                    codeStats->lineCount++;
                    codeStats->linesWithCodeCount += foundCodeOnLine;
                    foundCodeOnLine = 0;
                    state = START;
                }
                else if (ch == '/') // this is a second '/'
                {
                    codeStats->cplusplusCommentCount++;
                    state = INCPPCOMMENT;
                }
                else // it was only a single '/'
                {
                    foundCodeOnLine = 1;
                    state = START;
                }
                break; // break after case FOUND

            case INCPPCOMMENT:
                if(ch == '\n')
                {
                    codeStats->lineCount++;
                    codeStats->linesWithCodeCount += foundCodeOnLine;
                    foundCodeOnLine = 0;
                    state = START;
                }
                else
                {continue;}
                break; // break after case INCPPCOMMENT

            default:
                assert(0);
                break;
        }
    }
    fclose(f);
    assert(state == START);
}


int main(int argc, char *argv[])
{
    struct CodeStats codeStats;
    int i;

    for (i = 1; i < argc; i++) {
        codeStats_init(&codeStats);
        codeStats_accumulate(&codeStats, argv[i]);
        codeStats_print(codeStats, argv[i]); // no "&" -- see why?
        if (i != argc-1)   // if this wasn't the last file ...
            printf("\n");  // ... print out a separating newline
    }

    return 0;
}