#include <stdio.h>
#include <stdint.h>



void binprint(uint8_t data)
{
	int i;

	for (i=7 ; i >= 0 ; i--)
	{
		printf("%d", ((data & (1 << i))>0));
	}
}

main()
{
	binprint(0x01);
	printf("\n");
}
