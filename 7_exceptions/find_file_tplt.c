#define _GNU_SOURCE // to get asprintf() prototype
#include <stdio.h>  // this needs to be the first #include in that case

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <getopt.h>

#include "except.h"

/*
 * Here's how you declare an exception with the "except" package:
 */
Except_T StatFailed = { "Failed to open file." };
Except_T TargetFound = { "Target found." };
Except_T MiscellaneousError = { "Misc. error." };
/*
 * ASSIGNMENT
 *
 * Add additional declarations for exceptions TargetFound and
 * MiscellaneousError.
 */

char *progname = "*** progname not set ***"; /* should be argv[0] */

int verbose = 0; /* set on command line */

static void explore(char *path, char *target);

static void traverseDirectory(char path[], char *target) {
    DIR *dir = opendir(path);
    if (dir == NULL) { RAISE(MiscellaneousError); }
    //printf("path in traverseDir: %s\n", path);
    struct dirent *dir_struct = NULL;

    while ((dir_struct = readdir(dir)) != NULL) {
        if (strcmp(dir_struct->d_name, ".") == 0 || strcmp(dir_struct->d_name, "..") == 0) { continue; }

        char *subpath;
        asprintf(&subpath, "%s%s%s", path, "/", dir_struct->d_name);
        //printf("subpath: %s\n", subpath);
        TRY
                explore(subpath, target);
            EXCEPT(TargetFound)
                free(subpath);
                closedir(dir);
                RERAISE;
            ELSE
                fprintf(stderr, "explore() failed\n");
        END_TRY;
        free(subpath);
    }
    closedir(dir);

    /*
     * ASSIGNMENT
     *
     * Implement the following pseudocode:
     *
     * open the directory associated with `path` (hint: opendir(3))
     * if the open fails,
     *     raise the MiscellaneousError exception
     * for each entry in the directory (hint: readdir(3)),
     *     if the entry's name is "."  or ".."
     *         skip that entry
     *     allocate a string `subpath` concatenatiing `path`, "/", and
     *      the entry's name (hint: asprintf())
     *     if in the following ... (hint: TRY ... END_TRY)
     *         call explore on `subpath` and pass `target` as well
     *     ... the TargetFound exception is raised (hint: EXCEPT())
     *         free `subpath`
     *         close the open directory (hint: closedir(3)
     *         re-raise the exception (hint: RERAISE)
     *     ... any other exception is raised (hint: ELSE)
     *         print a message to stderr that explore() failed
     *     free `subpath`
     * close the directory associated with `path` (hint: closedir(3))
     */
}


static void explore(char *path, char *target)
/* look at, in, and below `path` for a file named `target` */
{
    struct stat sb;
    if(stat(path, &sb) == -1)
    {
        printf("couldn't open: %s\n", path);
        RAISE(StatFailed);
    }
    //printf("path in explore: %s\n", path);
    char* loc_rm_slash = strrchr(path, '/');

    if (loc_rm_slash == NULL)
        loc_rm_slash = path;
    else
        loc_rm_slash = loc_rm_slash +1;

    if(strcmp(loc_rm_slash, target) == 0)
    {
        if(verbose==1)
        {
            printf("%s\n", path);
        }
        RAISE(TargetFound);
    }

    if(S_ISDIR(sb.st_mode))
    {
        traverseDirectory(path, target);
    }

    /*
     * ASSIGNMENT
     *
     * Implement the following pseudocode:
     *
     * get the status of `path` (hint: stat(2))
     * if it fails,
     *     raise the StatFailed exception (hint: RAISE)
     * find the last '/'-delimited component of `path` (or use `path`
     *  itself if it contains no '/'s, hint: strrchr())
     * if that component is equal to `target`, (hint: strcmp())
     *     if `verbose` is set,
     *         print `path` to standard output, followed by a newline
     *          (hint: printf())
     *     raise the TargetFound exception (hint: RAISE())
     * if `path` is a directory (hint: S_ISDIR())
     *     traverse it (hint: traverseDirectory())
     */
}

void findFile(char *top, char *target)
{
    TRY
        explore(top, target);
    EXCEPT(StatFailed)
    ;
    EXCEPT(TargetFound)
    exit(0);
    END_TRY;
    /*
     * ASSIGNMENT
     *
     * Implement the following pseudocode:
     *
     * if in the following ... (hint: TRY ... END_TRY)
     *     call explore on `top` and pass `target` as well
     * ... the StatFailed exception is raised
     *     do nothing (put a ";" here)
     * ... the TargetFound exception is raised
     *     exit successfully (hint: exit(3))
     */
}


void usage(void)
{
    printf("usage: %s [-h] [-v] target [directory]*\n", progname);
}


int main(int argc, char *argv[])
{
    int i, ch;
    char *target;
    extern int optind;

    progname = argv[0];
    while ((ch = getopt(argc, argv, "hv")) != -1) {
        switch (ch) {

        case 'v':
            verbose = 1;
            break;

        case 'h':
            usage();
            exit(EXIT_SUCCESS);

        case '?':
            usage();
            exit(EXIT_FAILURE);
        }
    }
    if (optind >= argc) {
        usage();
        exit(EXIT_FAILURE);
    }
    target = argv[optind++];
    //printf("verbosity: %d\n", verbose);
    if (optind == argc) {
        /* directory name(s) not provided */
        findFile(".", target);
    } else {
        /* directory name(s) provided */
        for (i = optind; i < argc; i++)
            findFile(argv[i], target);
    }
    /*
     * If we find the target, we'll exit immediately (and
     * successfully), so if we get to this point...
     */
    exit(EXIT_FAILURE);
}
