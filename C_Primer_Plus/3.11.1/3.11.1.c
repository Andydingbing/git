#include <stdio.h>

int main()
{
	int a_int = 2147483647;    		 //2^31 - 1,最高位代表正负，31位1
	unsigned int a_uint = 4294967295;	 //2^32 - 1

	float a_float_max = 1.000E38;		 //8位指数部分，24位尾数部分
	float a_float_min = 0.1234E-38;

	printf("a_int = %d\n",a_int);
	printf("a_int + 1 = %d\n",a_int + 1);
	printf("a_uint = %u\n",a_uint);
	printf("a_uint + 1 = %u\n",a_uint + 1);
	printf("\n");
	printf("a_float_max = %e\n",a_float_max);	
	printf("a_float_max * 100.0f = %e\n",a_float_max * 100.0f);
	printf("a_float_min = %e\n",a_float_min);
	printf("a_float_min / 10.0f = %e\n",a_float_min / 10.0f);
	printf("\n");
	return 0;
}
