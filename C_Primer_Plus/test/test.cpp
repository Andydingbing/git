#include <stdio.h>
#include <unistd.h>

int main()
{
//	for (int i = 0;i < 10000; i ++) {
//		printf("%d\r",i);
//		fflush(stdout);
//		usleep(1000);
//	}
	int a = 2;
	int b = 3;
	int c = (a + b++) * 6;
	printf("c = %d\n",c);
	printf("b = %d\n",b);
	return 0;
}
