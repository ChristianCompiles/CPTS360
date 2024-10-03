// It's not a bad practice to list *why* you include particular
// headers.
#include <stdio.h> // for FILE, NULL, fopen(), and getc()
#include <string.h>
#include "eprintf.h" // for eprintf_fail()
#include <errno.h>
// Although not strictly required, its a good practice to include the
// header file that corresponds to this source file to detect any
// discrepancies between the function's declaration and its
// definition.
#include "compare_files.h"


int compareFiles(char *fname0, char *fname1)
{
    //
    // ASSIGNMENT
    //
    // This function compares two files named `fname0` and `fname1`
    // and returns true (1) if they are identical or false (0) if they
    // are not. Here's how it would be described in pseudocode (note
    // the indented block structure):
    //
    //   open file 0 for reading (hint: fopen(3))
    //   if the open fails,
    //       exit with an error message
    //   open file 1 for reading (hint: fopen(3))
    //   if the open fails,
    //       exit with an error message
    //   repeat until this function returns:
    //       read a character `ch0` from file 0 (hint: getc(3))
    //       read a character `ch1` from file 1 (hint: getc(3))
    //       compare both characters to each other and to `EOF`,
    //        (possibly) returning 0 or 1
    //
    // The last statement is intentionally vague. The logic here is
    // important. No extra points challenge: It can be done in a
    // single `if`-block.
    //
    FILE* file0 = fopen(fname0, "r");
    if(file0 == NULL)
    	{
		printf("File failed to open: %s\n", strerror(errno));
		return 0;
	}

    FILE* file1 = fopen(fname1, "r");  

    if(file1 == NULL)
	{
		printf("File failed to open: %s\n", strerror(errno));
		return 0;
	}

	int char0, char1;
	
	while(1)
	{
		char0 = fgetc(file0);
		char1 = fgetc(file1);
		
		if(char0 == EOF && char1 == EOF) // at the end of both files
			return 1;

		if(char0 != char1) // characters differ
		{
			return 0;
		}
	}
	int closefile = fclose(file0);
	
	if(closefile == EOF)
		printf("First file failed to close: %s\n", strerror(errno));

	closefile = fclose(file1);

	if(closefile == EOF)
		printf("Second file failed to close: %s\n", strerror(errno));
}	
