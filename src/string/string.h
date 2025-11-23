#ifndef STRING_H
#define STRING_H
int strlen(const char* ptr);
int strnlen(const char* ptr, int max);
int isdigit(char c);
int tonumericdigit(char c);
char* strcpy(char* dest, const char* src);
char tolower(char s);
char strncmp(const char* s1, const char* s2, int n);
int istrncmp(const char* s1, const char *s2, int n);
int strnlen_terminator(const char* str, int max, char terminator);
#endif
