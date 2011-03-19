#include <stdio.h>

void swap (int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}
  
int main()
{
	int v[40];
	v[20] = 20;
	v[21] = 21;
	swap(&v[20],&v[21]); 
	printf("v[20]: %d, v[21]: %d\n", v[20], v[21]);
	return 0;
}
