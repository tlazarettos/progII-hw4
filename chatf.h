#include <stdlib.h>
#include <string.h>

void failcheck(int rv, int line)
{
	if(rv<0)
	{
		fprintf(stderr,"%s: %s (Error in line: %d)\n", __FILE__, strerror(errno) , line);
		exit(-1);
	}
}

int mes


