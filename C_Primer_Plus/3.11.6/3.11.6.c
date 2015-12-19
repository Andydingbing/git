#include <stdio.h>

int main()
{
	double h2o_g = 3.0E-23;
	double water_kua_tuo = 0.0;
	long double number = 0;
	printf("Enter water:");
	scanf("%lf",&water_kua_tuo);
	number = 950.0 * water_kua_tuo / h2o_g;
	printf("The water has %Lf number of molecules.\n",number);
	printf("The water has %Le number of molecules.\n",number);
	return 0;
}
