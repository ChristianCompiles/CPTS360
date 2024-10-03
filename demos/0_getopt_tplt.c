#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

static char *progname = NULL;

static void doit(FILE *fp);
static void usage(void);

int main(
	int argc,
	char **argv)
{
	int ch;
	extern char *optarg;
	extern int optind;
	int ok = 1;
	FILE *fpIn;

	progname = argv[0];
	while ((ch = getopt(argc, argv, "im:h")) != -1) {
		switch (ch) {

		case 'i':
			break;

		case 'm':
			/* following argument is in "optarg" */
			break;

		case 'h':
			usage();
			exit(0);
		}
	}
	if (!ok) {
		usage();
		exit(1);
	}
	if (optind == argc) {
		doit(stdin);
	} else {
		for (; optind < argc; optind++) {
			fpIn = fopen(argv[optind], "r");
			if (fpIn == NULL) {
				(void) fprintf(stderr, "can\'t open \"%s\" for reading\n",
						argv[optind]);
				exit(1);
			} else {
				doit(fpIn);
				fclose(fpIn);
			}
		}
	}
	exit(0);
}

static void doit(
	FILE *fp)
{
	return;
}

/* usage: issue a usage error message */
static void usage(void)
{
	(void) fprintf(stderr,
			"usage: %s [{args}] [{data file name}]\n", progname);
	(void) fprintf(stderr, "%s\n",
			" {args} are:");
	(void) fprintf(stderr, "%s\n",
			"  -h             this help message");
	return;
}
