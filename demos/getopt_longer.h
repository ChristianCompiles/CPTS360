#ifndef _GETOPT_LONGER_INCLUDED
#define _GETOPT_LONGER_INCLUDED

#include <getopt.h>

// Use these mnemonically for the has_arg member of the option struct
// ("opt").
enum {
    NO_ARG  = 0,  // the option takes no argument
    REQ_ARG = 1, // the option must have an argument
    OPT_ARG = 2  // the option takes an optional argument
};

// We "wrap" the standard getopt "struct option" with the additional
// information we provide.
struct option_help {
    struct option opt;
    char *help;  // help information pertaining to this particular option
};

/* in "getopt_longer" */
extern int getopt_longer(int argc, char * const argv[],
                    const struct option_help *opth, int *longindex);
extern void opth_printUsage(const struct option_help *opth, const char *progname);

#endif /* _GETOPT_LONGER_INCLUDED */
