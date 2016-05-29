/*************************************************************************
	> File Name: tunable.h
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Wed 25 May 2016 08:24:47 AM PDT
 ************************************************************************/

#ifndef _TUNABLE_H_
#define _TUNABLE_H_

// bool if on(1) or off(0)
extern int tunable_pasv_enable;
extern int tunable_port_enable;

// int value
extern int tunable_listen_port;

extern int tunable_max_clients;
extern int tunable_max_per_ip;


extern int tunable_accept_timeout;
extern int tunable_connect_timeout;

extern int tunable_idle_session_timeout;
extern int tunable_data_connection_timeout;

extern int tunable_local_umask;
extern int tunable_upload_max_rate;
extern int tunable_download_max_rate;

extern const char * tunable_listen_address;
#endif // _TUNABLE_H_
