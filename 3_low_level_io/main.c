#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "eprintf.h"

int main(int argc, char *argv[])
{
    if(argc != 4) // make sure proper number of arguments are supplied
        eprintf_fail("Improper number of CL arguments\n");

    int buffer_size = atoi(argv[1]);
    if(buffer_size == 0)
        eprintf_fail("Invalid buffer_size\n");
    //printf("buffer_size: %d\n", buffer_size); // for testing atoi

    char* cpy_buffer = (char*)malloc(buffer_size * sizeof(char));
    if(cpy_buffer == NULL)
        eprintf_fail("Error creating copy buffer: %s\n", strerror(errno));

    //printf("trying to read from: %s\n", argv[2]);

    int reading_descr = open(argv[2], O_RDONLY); // | S_IRUSR);
    if(reading_descr == -1)
        eprintf_fail("Error reading file: %s\n", argv[2]);

    int writing_descr = open(argv[3], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if(writing_descr == -1)
        eprintf_fail("Error reading file: %s,\n %s\n", argv[3], strerror(errno));
    
    int amount_read = 0;
    int amount_write = 0;

    while(1)
    {
        amount_read = read(reading_descr, cpy_buffer, buffer_size);
        if(amount_read == 0)
            break;
        if(amount_read == -1)
            eprintf_fail("Failed during read\n%s", strerror(errno));

        amount_write = write(writing_descr, cpy_buffer, amount_read);
        if(amount_write == -1)
            eprintf_fail("Failed during write\n%s", strerror(errno));
    }

    free(cpy_buffer);

    int closefile = close(reading_descr);
	
	if(closefile == -1)
		printf("First file failed to close: %s\n", strerror(errno));

	closefile = close(writing_descr);

	if(closefile == -1)
		printf("Second file failed to close: %s\n", strerror(errno));

    return 0;
}