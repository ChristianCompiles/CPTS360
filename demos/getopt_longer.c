#include <stdio.h>
#include <string.h>

#include "getopt_longer.h"
#include "allocarray.h"

// Note that we construct the 'optstring' and the 'struct opt' array
// that 'getopt_long()' expects every time this function is called.
// This provides consistency ("DRY") and flexibility at the expense of
// efficiency, but in practice option processing should be a very
// small fraction of execution time.

// getopt_longer -- enhanced version of getopt_long()
int getopt_longer(int argc, char * const argv[],
                    const struct option_help *opth, int *longindex)
{
    int i, j, k;
    char *optstring;
    int result;
    struct option *opt;

    ALLOC_ARRAY(optstring, char, 1);
    j = 0;
    optstring[j] = '\0';
    k = 0;
    ALLOC_ARRAY(opt, struct option, 1);
    for (i = 0;
         opth[i].opt.name || opth[i].opt.has_arg || opth[i].opt.flag || opth[i].opt.val;
         i++) {
        if (opth[i].opt.val) {
            REALLOC_ARRAY(optstring, char, j+2); // allow null-termination
            optstring[j++] = opth[i].opt.val;
            optstring[j] = '\0';
            switch (opth[i].opt.has_arg) {
            case NO_ARG:
                break;
            case OPT_ARG: 
                // append "::"
                REALLOC_ARRAY(optstring, char, j+3);
                optstring[j++] = ':';
                optstring[j++] = ':';
                optstring[j] = '\0';
                break;
            case REQ_ARG: 
                // append ":"
                REALLOC_ARRAY(optstring, char, j+2);
                optstring[j++] = ':';
                optstring[j] = '\0';
                break;
            default: assert(0);
            }
        }
        if (opth[i].opt.name) {
            REALLOC_ARRAY(opt, struct option, k+1);
            opt[k++] = opth[i].opt;
        }
    }
    // "null" terminate opt[] the way getopt_long() is expecting
    REALLOC_ARRAY(opt, struct option, k+1);
    opt[k] = opth[i].opt;
    result = getopt_long(argc, argv, optstring, opt, longindex);
    FREE_ARRAY(optstring);
    FREE_ARRAY(opt);
    return result;
}

// opth_printUsage -- construct and print a usage message to standard error
void opth_printUsage(const struct option_help *opthBase, const char *progname)
{
    const struct option_help *opth;
    char *arginfoLong, *arginfoShort;

	fprintf(stderr,
        "usage: %s [{option}]* {file}\n"
            "  {option} is:\n"
            "\n",  progname);
    for (opth = opthBase;
         opth->opt.name || opth->opt.has_arg
             || opth->opt.flag || opth->opt.val;
         opth++) {
        // select how to indicate no, an optional, or a required option argument 
        switch (opth->opt.has_arg) {

        case NO_ARG:
            arginfoLong = "";
            arginfoShort = "";
            break;

        case REQ_ARG:
            arginfoLong = "={argument}";
            arginfoShort = "{argument}";
            break;

        case OPT_ARG:
            arginfoLong = "[={argument}]?";
            arginfoShort = "[{argument}]?";
            break;

        default:
            assert(0);
        }
        if (opth->opt.name && opth->opt.name[0] && opth->opt.val) {
            // there's both a long and a short form of the option
            if (strlen(arginfoShort) == 0) {
                fprintf(stderr, "--%s%s (or -%c)\n",
                        opth->opt.name, arginfoLong,
                        opth->opt.val);
            } else {
                fprintf(stderr, "--%s%s (or -%c%s or -%c %s)\n",
                        opth->opt.name, arginfoLong,
                        opth->opt.val, arginfoShort,
                        opth->opt.val, arginfoShort);
            }
        } else if (opth->opt.name && opth->opt.name[0]) {
            // there's only a long form of the option
            fprintf(stderr, "--%s%s\n",
                    opth->opt.name, arginfoLong);
        } else if (opth->opt.val) {
            // there's only a short form of the option
            if (strlen(arginfoShort) == 0) {
                fprintf(stderr, "-%c\n", opth->opt.val);
            } else {
                fprintf(stderr, "-%c%s or -%c %s\n",
                        opth->opt.val, arginfoShort,
                        opth->opt.val, arginfoShort);
            }
        }
        if (opth->help)
            fprintf(stderr, "%s", opth->help);
    }
	fprintf(stderr,
            "\n"
			"syntax: [{anything}]* is 0 or more {anything}s.\n"
			"        [{anything}]? is an optional {anything}.\n"
        );
}
