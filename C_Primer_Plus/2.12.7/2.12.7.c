#include <stdio.h>

void one_three();
void two();

int main()
{
	printf("starting now:");
	printf("\n");
	one_three();
	printf("\n");
	two();
	printf("\nthree\n");
	printf("done!");
	printf("\n");
}

void one_three()
{
	printf("one");
}

void two()
{
	printf("two");
}
