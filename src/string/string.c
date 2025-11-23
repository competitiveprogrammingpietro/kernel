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


char* strcpy(char* dest, const char* src)
{
	while (*src != 0)
	{
		*dest = *src;
		src++;
		dest++;
	}

	*dest = 0;
	return dest;
}

char tolower(char s)
{
	if (s < 65 || s > 90)
	{
		return s;
	}
	return s + 32;
}

char strncmp(const char* s1, const char* s2, int n)
{
	while (n > 0)
	{
		unsigned char c1 = (unsigned char) *s1;
		unsigned char c2 = (unsigned char) *s2;

		if (c1 != c2)
		{
			return c1 - c2;
		}

		if( c1 == '\0')
		{
			return 0; // Equals
		}

		n--;
		s1++;
		s2++;
	}
	return 0; // Should never end up here if they are NULL terminated
}

// Same as strncmp but it is case insensitive
int istrncmp(const char* s1, const char *s2, int n)
{
	while (n > 0)
	{
		unsigned char c1 = (unsigned char) tolower(*s1);
		unsigned char c2 = (unsigned char) tolower(*s2);

		if (c1 != c2)
		{
			return c1 - c2;
		}

		if( c1 == '\0')
		{
			return 0; // Equals
		}

		n--;
		s1++;
		s2++;
	}
	return 0; // Should never end up here if they are NULL terminated
}

int strnlen_terminator(const char* str, int max, char terminator)
{
	int i = 0;
	for (i = 0; i < max; i++)
	{
		if (str[i] == '\0' || str[i] == terminator)
			return i;
	}
	return i;

}