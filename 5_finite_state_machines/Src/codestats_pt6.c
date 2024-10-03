#include <stdio.h>
#include <assert.h> // for assert()
#include <ctype.h> // for isspace()

struct CodeStats {
    int lineCount;
    int linesWithCodeCount;
    int cplusplusCommentCount;
    int cCommentCount;
};


void codeStats_init(struct CodeStats *codeStats)
{
    codeStats->lineCount = 0;
    codeStats->linesWithCodeCount = 0;
    codeStats->cplusplusCommentCount = 0;
    codeStats->cCommentCount = 0;
}


void codeStats_print(struct CodeStats codeStats, char *fileName)
{
    printf("     file: %s\n", fileName);
    printf("     line count: %d\n", codeStats.lineCount);
    printf("     lines with code count: %d\n", codeStats.linesWithCodeCount);
    printf("     cpluspluscommentcount: %d\n", codeStats.cplusplusCommentCount);
    printf("     cCommentCount: %d\n", codeStats.cCommentCount);
}


void codeStats_accumulate(struct CodeStats *codeStats, char *fileName)
{
    FILE *f = fopen(fileName, "r");
    int ch;
    enum {
        START,
        FOUNDSLASH,
        INCPPCOMMENT,
        INCCOMMENT,
        FOUNDSTARINCCOMMENT,
        INSTRING,
        FOUNDESCAPEINSTRING,
        INCHARCONSTANT,
        FOUNDESCAPEINCHARCONSTANT,
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
                if(ch =='\"')
                {
                    foundCodeOnLine = 1;
                    state = INSTRING;
                    continue;
                }
                if(ch == '\'')
                {
                    foundCodeOnLine = 1;
                    state = INCHARCONSTANT;
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
                else if (ch == '*')
                {
                    codeStats->cCommentCount++;
                    state = INCCOMMENT;
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

            case INCCOMMENT:
                if(ch == '*')
                {
                    state = FOUNDSTARINCCOMMENT;
                }
                else if(ch == '\n')
                {
                    codeStats->lineCount++;
                    codeStats->linesWithCodeCount += foundCodeOnLine;
                    foundCodeOnLine = 0;
                }
                else
                {continue;}
                break; // break after case INCCOMMENT

            case FOUNDSTARINCCOMMENT:
                if(ch == '/')
                {
                    state = START;
                }
                else if(ch == '\n')
                {
                    codeStats->lineCount++;
                    codeStats->linesWithCodeCount += foundCodeOnLine;
                    foundCodeOnLine = 0;
                    state = INCCOMMENT;
                }
                else
                {
                    state = INCCOMMENT;
                }
                break; // break after case FOUNDSTARINCCOMMENT

            case INSTRING:
                if(ch == '"')
                {
                    state = START;
                    continue;
                }
                if(ch == '\\')
                {
                    state = FOUNDESCAPEINSTRING;
                    continue;
                }
                break; // break after case INSTRING
            case FOUNDESCAPEINSTRING:
                if(ch == '\n')
                {
                    codeStats->lineCount++;
                    codeStats->linesWithCodeCount += foundCodeOnLine;
                    foundCodeOnLine = 1;
                    state = INSTRING;
                }
                else
                {
                    state = INSTRING;
                }
                break; // break case after FOUNDESCAPEINSTRING

            case INCHARCONSTANT:
                if(ch == '\'')
                {
                    state = START;
                    continue;
                }
                if(ch == '\\')
                {
                    state = FOUNDESCAPEINCHARCONSTANT;
                    continue;
                }
                break; // break case after INCHARCONSTANT

            case FOUNDESCAPEINCHARCONSTANT:
                if(ch == '\n')
                {
                    codeStats->lineCount++;
                    codeStats->linesWithCodeCount += foundCodeOnLine;
                    foundCodeOnLine = 1;
                    state = INCHARCONSTANT;
                }
                else
                {
                    state = INCHARCONSTANT;
                }
                break; // break case after FOUNDESCAPEINCHARSCONSTANT
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