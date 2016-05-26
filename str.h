/*************************************************************************
	> File Name: str.h
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Wed 25 May 2016 07:07:26 AM PDT
 ************************************************************************/

#ifndef _STR_H_
#define _STR_H_
#include "common.h"
void str_trim_crlf(char * str);

void str_split(const char * str, char * left, char * right, char c);

void str_to_upper(char * str);

long long str_to_ll(const char * str);

unsigned int str_octal_to_uint(const char * str);

int str_all_space(const char * str);

#endif // _STR_H_
