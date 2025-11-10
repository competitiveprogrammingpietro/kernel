#include "string.h"

int strlen(const char* ptr)
{
	int size = 0;
	while (*ptr)
	{
		size++;
		ptr++;
	}
	
	return size;
}


int strnlen(const char* ptr, int max)
{
	int size = 0;
	for (size = 0; size < max; size++)
	{
		size++;
		ptr++;
	}
	return size;
}


int isdigit(char c)
{
	return c >= 48 && c <= 57;
}

// From the ASCII code point to the number, let's say
// '0' is 48, means, 48 - 48 = 0
int tonumericdigit(char c)
{
	return c-48;
}
