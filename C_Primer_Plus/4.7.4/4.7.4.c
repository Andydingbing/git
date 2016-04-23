#include <stdio.h>

#define B "booboo"
#define X 10

int main()
{
	int age;
	int xp;
	char name[64];

	printf("Please enter your first name.");
	scanf("%s",name);
	printf("ALl rigth, %s, what's your age?\n", name);
	scanf("%d",&age);
	xp = age + X;
	printf("That's a %s! You must be at least %d.\n", B, xp);
	return 0;
}
