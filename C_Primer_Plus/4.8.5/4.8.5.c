#include <stdio.h>
#include <string.h>

int main()
{
	char surname[32];
	char name[32];
	int  size = 0;
	printf("Please enter the name:");
	scanf("%s",name);
	size = (int)strlen(name);
	printf("Please enter the surname:");
	scanf("%s",surname);
	printf("%*s %10s\n", size, name, surname);
	printf("%*d %10d\n\n", size, strlen(name), strlen(surname));
	printf("%*s %10s\n", size, name, surname);
	printf("%*d %-10d\n", (size * -1), strlen(name), strlen(surname));
	return 0;
}
