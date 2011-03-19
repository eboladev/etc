#include <stdio.h>
int main()
{
	extern int __argc;
	extern char **__argv;
	int i;
	for (i = 0; i < __argc; i++)
		printf("%s\n", __argv[i]);
	return 0;
}
