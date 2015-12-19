#include <stdio.h>

int main()
{
	float a = 0.0f;
	printf("Enter a float:");
	scanf("%f",&a);
	printf("The input is %.6f or %.6e\n",a,a);
	return 0;
}
