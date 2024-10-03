#include <stdio.h>
#include "hello.h"

#define maxnamesize 100
void activate_new_feature() {
	printf("Activating new feature...\n");
	// This either will look blue when printed or just look like a bunch of weird text
	// if your terminal somehow doesn't support ANSI codes.
	printf("\x1b[36mNew Feature activated\x1b[0m\n");
}

int main(int argc, char** argv) {
	
	activate_new_feature();
	char buffer[maxnamesize];
	printf("Enter your name: ");
	fgets(buffer, maxnamesize, stdin);
	say_hello(buffer);
}
