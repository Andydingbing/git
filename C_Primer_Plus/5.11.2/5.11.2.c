#include <stdio.h>

int main()
{
	int a = 0;
	printf("Entet the start integer:");
	scanf("%d",&a);
	for (int i = 0;i < 11;i ++) {
		printf("%d\t", a++);
	}
	printf("\n");
	return 0;
}
