/*************************************************************************
	> File Name: parseconf.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Wed 25 May 2016 06:49:46 AM PDT
 ************************************************************************/

#include "parseconf.h"
#include "common.h"
#include "str.h"
#include "tunable.h"

struct parseconf_uint_setting {
	char * p_setting_name;
	int * p_variable;
};

parseconf_uint_setting parseconf_uint_array[] = 
{
	{"listen_port", &tunable_listen_port},
	{NULL, NULL}
};


// parse the configure file
// file format
// name=value
// # stands for commonts
void parseconf_load_file(const char * path) {
	FILE * fp = fopen(path, "r");
	if (fp == NULL) {
		ERR_EXIT("fopen");
	}

	char setting_line[1024]; // declare
	memset(setting_line, 0, sizeof(setting_line)); // initialization

	while (fgets(setting_line, sizeof(setting_line), fp) != NULL) {
		// empty line or comment line skip
		if (strlen(setting_line) == 0 || setting_line[0] == '#' || str_all_space(setting_line)) {
			continue;
		}
        // delete the \r\n
		str_trim_crlf(setting_line);
		parseconf_load_setting(setting_line);
		memset(setting_line, 0, sizeof(setting_line));
	}
}

// parse specific line
void parseconf_load_setting(char * setting_line) {
        // skip the white space
		while (*setting_line == ' ') setting_line++;

		char key[128];
		char value[128];
		memset(key, 0, sizeof(key));
		memset(value, 0, sizeof(value));

		str_split(setting_line, key, value, '=');
		if (strlen(value) == 0) {
			ERR_EXIT("missing value in config file for ");
		}
	   
		// parse int value in the configure file
		parseconf_uint_setting * p_uint_setting = parseconf_uint_array;
		while (p_uint_setting->p_setting_name != NULL) {
			if (strcmp(p_uint_setting->p_setting_name, key) == 0) { // match
				if (value[0] = '0') { // 8 
					*(p_uint_setting->p_variable) = str_octal_to_uint(value);
				} else {
					*(p_uint_setting->p_variable) = atoi(value);
				}
				break;
			}
			p_uint_setting++;
		}
}

