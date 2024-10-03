#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

// This is a general-purpose main() template that makes use of the
// getopt_long() utility function to process its arguments.  As it's a
// template, make a copy of it and change it as needed, stripping out
// or modifying all the irrelevant code and comments.  (Such as this
// one!)

// The general syntax this template follows is (regular expression):
//  $ {progname} [{arg}]* [{filename}]*
// If {filename} is omitted, it gets its input from standard input.

// forward declarations
static void doit(FILE *f);
static void usage(char *progname);

// To get getopt_long() to work, you need to provide a static
// (usually) array of "struct option" structures.  There are four
// members to be filled in:

// 1. The name is a (char *) string containing the "long" option name
// (e.g. "--help" or "--format=pdf").

// 2. The "has_arg" member of a "struct option" has one of these
// values that describe the corresponding option:
enum {
    NO_ARG  = 0,  // the option takes no argument
    REQ_ARG = 1, // the option must have an argument
    OPT_ARG = 2  // the option takes an optional argument
};

// 3. The "flag" is an int pointer that determines how the function
// will return its value.  If it is NULL, getopt_long() will return
// "val" (the fourth member) as its function return.  If it is not
// NULL, getopt_long() will return 0 and set "*flag" to "val".

// 4. "val" is an int which is either a character to denote a "short"
// (e.g. "-h" or "-f pdf") option or 0, to denote an option which does
// not have a "short" form.

// The array is terminated by an entry "{ NULL, NO_ARG, NULL, 0 }".

static struct option opt[] = {
    // elements are:
    // name       has_arg   flag   val
    { "add",      REQ_ARG,  NULL,   0 },
    { "append",   OPT_ARG,  NULL,   0 },
    { "delete",   REQ_ARG,  NULL,   0 },
    { "help",     NO_ARG,   NULL,  'h'}, // "--help" aliased to short "-h"
    { "verbose",  NO_ARG,   NULL,   0 },
    { "create",   REQ_ARG,  NULL,  'c'}, // "--create" aliased to short "-c"
    { "",         NO_ARG,   NULL,  'H'}, // short-only option (undocumented)
    { "file",     REQ_ARG,  NULL,   0 },
    { NULL,       NO_ARG,   NULL,   0 }  // end of opt table
    // note: All "flag"s == NULL means that "getopt_long()" will
    // return "val" for all options.
};

int main(int argc, char **argv)
{
    int c;
    int digit_optind = 0;
    FILE *fIn;
    static char *progname = "**UNSET**";

    progname = argv[0];
    for (;;) {
        int this_option_optind = optind ? optind : 1;
        int iOpt = 0;

        c = getopt_long(argc, argv, "abc:d:e::h012", opt, &iOpt);
        if (c == -1)
            break;

        switch (c) {

        case 0:
             // This branch is taken for all options that have val ==
             // 0.  These are options which accept long names /only/.
            printf("option %s", opt[iOpt].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;

        case '0':
        case '1':
        case '2':
            // allow one of these to be set
            if (digit_optind != 0 && digit_optind != this_option_optind)
                printf("digits occur in two different argv-elements.\n");
            digit_optind = this_option_optind;
            printf("option %c\n", c);
            break;

        case 'a':
            printf("option a\n");
            break;

        case 'b':
            printf("option b\n");
            break;

        case 'c':
            printf("option c with value `%s'\n", optarg);
            break;

        case 'd':
            printf("option d with value `%s'\n", optarg);
            break;

        case 'e':
            if (optarg)
                printf("option e with value `%s'\n", optarg);
            else
                printf("option e with optional value defaulted\n");
            break;

        case 'h':
			usage(progname);
			exit(0);

        default:
            printf("?? getopt returned character code 0x%02x ??\n", c);
			exit(1);
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

/* usage: issue a usage error message */
static void usage(char *progname)
{
	(void) fprintf(stderr,
			"usage: %s [{args}] [{data file name}]\n", progname);
	(void) fprintf(stderr, "%s\n",
			" {args} are:");
	(void) fprintf(stderr, "%s",
                   "  -a             option a\n"
                   "  -b             option b\n"
                   "  -c {arg}       option c (takes argument)\n"
                   "  -d {arg}       option d (takes argument)\n"
                   "  -e [{arg}]     option d (takes optional argument)\n"
                   "  -h             help (this message) and exit\n"
                   "  -0             option 0\n"
                   "  -1             option 1\n"
                   "  -2             option 2\n");
	return;
}
