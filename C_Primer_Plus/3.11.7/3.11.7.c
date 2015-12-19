#include <stdio.h>

int main()
{
	float height = 0.0;
	printf("Enter your height(cm):");
	scanf("%f",&height);
	printf("Your height:%fcm = %finch.\n",height,height / 2.54);
	return 0;
}
