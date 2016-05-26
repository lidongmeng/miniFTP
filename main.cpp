/*************************************************************************
	> File Name: main.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Wed 25 May 2016 06:16:09 AM PDT
 ************************************************************************/
#include "common.h"
#include "parseconf.h"
#include "tunable.h"
int main() {
	// the ftp server should be run from the root
	if (getuid() != 0) {
		ERR_EXIT("miniftp: must be started as root\n");
	}
	// load configure file
	parseconf_load_file(CONF_FILE_NAME);

	printf("tunable_listen_port=%d\n", tunable_listen_port);
	// establish a socket in the server
	// bind it to a port: port can be parse from the configure file
	return 0;
}
