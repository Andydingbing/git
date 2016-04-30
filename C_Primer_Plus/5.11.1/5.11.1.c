#include <stdio.h>

#define One_Hour 60

int main()
{
	int min = 1;
	int hour = 0;
	int min_left = 0;
	while(min > 0) {
		printf("Enter the minutes being to be converted:");
		scanf("%d",&min);
		if (min <= 0) 
			break;
		hour = min / One_Hour;
		min_left = min % One_Hour;
		printf("%d minutes = %dHour & %d minutes.\n\n", min, hour, min_left);
	}
	return 0;
}
