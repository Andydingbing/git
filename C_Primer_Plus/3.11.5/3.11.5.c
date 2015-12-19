#include <stdio.h>

int main()
{
	int age = 0;
	unsigned long long int year_second = (int)3.156E7;
	printf("Enter your age:");
	scanf("%d",&age);
	printf("You have lived %lld seconds.\n",age * year_second);
	return 0;
}
