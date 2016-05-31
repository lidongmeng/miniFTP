/*************************************************************************
	> File Name: str.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Wed 25 May 2016 07:10:32 AM PDT
 ************************************************************************/

#include "str.h"
#include "common.h"

// delete the \r\n of the end of a line
void str_trim_crlf(char * str) {
	int len = strlen(str);
	char * p = &str[len - 1];
	while (*p == '\r' || *p == '\n') *p-- = '\0';
}
// split the line by char(c) and store l int the left and r in the right
void str_split(const char * str, char * left, char * right, char c) {
	// find the c int the str
	char * p = strchr((char*)str, c);

	if (p == NULL) {
		strcpy(left, str);
	} else {
		strncpy(left, str, p-str);
		strcpy(right, p+1);
	}
}

// if a line contains all space
int str_all_space(const char * str) {
	while (*str) {
		if (!isspace(*str)) return 0;
		++str;
	}
	return 1;
}

// translate str to upper case
void str_to_upper(char * str) {
	while (*str) {
		*str = toupper(*str);
		++str;
	}
}

// parse the octal number to unsigned int
unsigned int str_octal_to_uint(const char * str) {
	printf("str_octal_to_uint: %s\n", str);
	unsigned int result = 0;
	while (*str && *str == '0') ++str;
	while (*str) {
		int digit = *str;
		if (!isdigit(digit) || digit > '7') break;
		
		result <<= 3;
		result |= (digit - '0');
		++str;
	}
	return result;
}

// parse number to long long
long long str_to_ll(const char * str) {
	long long result = 0;
	while (*str) {
		if (*str < '0' || *str > '9') return 0;
		int temp =int ((*str) - '0');
		result = result*10 + temp;
		++str;
	}
	return result;
}

