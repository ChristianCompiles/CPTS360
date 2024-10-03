#include <stdio.h>
#include <stdlib.h>

#include "getopt_longer.h"

// This is a general-purpose main() template that makes use of the
// getopt_longer() utility function to process its arguments.  As it's
// a template, make a copy of it and change it as needed, stripping
// out or modifying all the irrelevant code and comments.  (Such as
// this one!)

// The general syntax this template follows is (regular expression):
//  $ {progname} [{arg}]* [{filename}]*
// If {filename} is omitted, it gets its input from standard input.

// forward declarations
static void doit(FILE *f);

static struct option_help opth[] = {
    // elements are:
    // name       has_arg   flag   val
    { { "",         NO_ARG,   NULL,  's'}, // short-only option (undocumented)
      "    Don't do anything long.\n"
    },
    { { "flummox",  REQ_ARG,  NULL,   'f' },
      "    Flummox the instabulators using {argument}.\n"
    },
    { { "greeble",  OPT_ARG,  NULL,  'g' },
      "    Greeble the doodles, with {argument}, if provided (default: snurgles).\n"
    },
    { { "help",     NO_ARG,   NULL,  'h'}, // "--help" aliased to short "-h"
      "    Print out (this) usage information.\n"
    },
    { { "output",   REQ_ARG,  NULL,   'o' },
      "    Direct the output to {argument}.\n"
    },
    { { "verbose",  NO_ARG,   NULL,   0 },
      "    Be verbose.  Be very verbose.\n"
    },
    { { NULL,       NO_ARG,   NULL,   0 },  // end of opth table
      NULL }
    // note: All "flag"s == NULL means that "getopt_long()" will
    // return "val" for all options.
};

int main(int argc, char **argv)
{
    int c;
    FILE *fIn;

    for (;;) {
        int iOpt = 0;

        c = getopt_longer(argc, argv, opth, &iOpt);
        if (c == -1)
            break;

        switch (c) {

        case 0:
             // This branch is taken for all options that have val ==
             // 0.  These are options which accept long names /only/.
            printf("option %s", opth[iOpt].opt.name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;

        case 'f':
            break;

        case 'g':
            break;

        case 'o':
            break;

        case 'h':
			opth_printUsage(opth, argv[0]);
			exit(EXIT_SUCCESS);

        case 's':
            break;

        default:
            printf("?? getopt returned character code 0x%02x ??\n", c);
			exit(EXIT_FAILURE);
        }
    }

	if (optind == argc) {
		doit(stdin);
	} else {
		for (; optind < argc; optind++) {
			fIn = fopen(argv[optind], "r");
			if (fIn == NULL) {
				(void) fprintf(stderr, "can\'t open \"%s\" for reading\n",
						argv[optind]);
				exit(EXIT_FAILURE);
			} else {
				doit(fIn);
				fclose(fIn);
			}
		}
	}

    exit(EXIT_SUCCESS);
}

static void doit(
	FILE *f)
{
	return;
}
