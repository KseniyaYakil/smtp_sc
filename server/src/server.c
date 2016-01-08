#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc == 1) {
		perror("1");
		return -1;
	}

	(void)argv;
	return 0;
}
