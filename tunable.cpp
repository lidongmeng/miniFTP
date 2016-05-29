/*************************************************************************
	> File Name: tunable.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Wed 25 May 2016 06:15:01 PM PDT
 ************************************************************************/
#include "tunable.h"

// bool if on(1) or off(0)
int tunable_pasv_enable = 1;
int tunable_port_enable = 1;

// int value
int tunable_listen_port = 5553;
int tunable_max_clients = 2000;
int tunable_max_per_ip = 50;

int tunable_accept_timeout = 60;
int tunable_connect_timeout = 60;
int tunable_idle_session_timeout = 300;
int tunable_data_connection_timeout = 300;

int tunable_local_umask = 077;
int tunable_upload_max_rate = 0;
int tunable_download_max_rate = 0;

 const char * tunable_listen_address;

