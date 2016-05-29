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

struct parseconf_bool_setting {
	char * p_setting_name;
	int * p_variable;
};

parseconf_bool_setting parseconf_bool_array[] = 
{
	{"pasv_enable", &tunable_pasv_enable},
	{"port_enable", &tunable_port_enable},
	{NULL, NULL}
};

struct parseconf_uint_setting {
	char * p_setting_name;
	int * p_variable;
};

parseconf_uint_setting parseconf_uint_array[] = 
{
	{"listen_port", &tunable_listen_port},
	{"max_clients", &tunable_max_clients},
	{"max_per_ip", &tunable_max_per_ip},
	{"accept_timeout", &tunable_accept_timeout},
	{"connect_timeout", &tunable_connect_timeout},
	{"idle_session_timeout", &tunable_idle_session_timeout},
	{"data_connection_timeout", &tunable_data_connection_timeout},
	{"local_umask", &tunable_local_umask},
	{"upload_max_rate", &tunable_upload_max_rate},
	{"download_max_rate", &tunable_download_max_rate},
	{NULL, NULL}
};

struct parseconf_string_setting {
	char * p_setting_name;
	const char ** p_variable;
};

parseconf_string_setting parseconf_string_array[] = 
{
	{"listen_address", &tunable_listen_address},
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
		//	printf("setting_line=%s\n", setting_line);

		parseconf_load_setting(setting_line);
		memset(setting_line, 0, sizeof(setting_line));
	}
}

// parse specific line
void parseconf_load_setting(char * setting_line) {
        // skip the white space
		while (*setting_line == ' ') setting_line++;
	//	printf("setting_line=%s\n", setting_line);
		char key[128];
		char value[128];
		memset(key, 0, sizeof(key));
		memset(value, 0, sizeof(value));

		str_split(setting_line, key, value, '=');
	//	printf("key=[%s], value=[%s]\n", key, value);
		if (strlen(value) == 0) {
			ERR_EXIT("missing value in config file for ");
		}
	    
		// parse bool value in the configure file
		parseconf_bool_setting * p_bool_setting = parseconf_bool_array;
		while (p_bool_setting->p_setting_name != NULL) {
			if (strcmp(p_bool_setting->p_setting_name, key) == 0) { // match
				str_to_upper(value);
				if (strcmp(value, "YES") == 0 || strcmp(value, "TRUE") == 0 || strcmp(value, "1") == 0)
					*(p_bool_setting->p_variable) = 1;
			    else if (strcmp(value, "NO") == 0 || strcmp(value, "FALSE") == 0 || strcmp(value, "0") == 0)
					*(p_bool_setting->p_variable) = 0;
				else
					ERR_EXIT("bad bool value int the config file");
			}
			p_bool_setting++;
		}
     
		// parse string value in the configure file
		parseconf_string_setting * p_string_setting = parseconf_string_array;
		while (p_string_setting->p_setting_name != NULL) {
			if (strcmp(p_string_setting->p_setting_name, key) == 0) { // match
				const char ** p_cur_setting = p_string_setting->p_variable;
				if (*p_cur_setting) free((char*)*p_cur_setting);
				*p_cur_setting = strdup(value);
			}
			p_string_setting++;
		}
   
		// parse int value in the configure file
		parseconf_uint_setting * p_uint_setting = parseconf_uint_array;
		while (p_uint_setting->p_setting_name != NULL) {
			if (strcmp(p_uint_setting->p_setting_name, key) == 0) { // match
			//	printf("value:%s\n", value);
				if (value[0] == '0') { // 8 
					*(p_uint_setting->p_variable) = str_octal_to_uint(value);
				} else {
					*(p_uint_setting->p_variable) = atoi(value);
				}
				break;
			}
			p_uint_setting++;
		}
}

