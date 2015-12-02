#include <stdio.h>

int printmessagea();
int printmessageb();

int main()
{
	printmessagea();
	printmessageb();
	return 0;
}

int printmessagea()
{
	for (int i = 0;i < 3;i ++)
		printf("For he's a jolly good fellow!\n");
	return 0;
}

int printmessageb()
{
	printf("Which nobody can deny!\n");
	return 0;
}
