/*************************************************************************
	> File Name: session.h
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Fri 27 May 2016 07:20:32 AM PDT
 ************************************************************************/

#ifndef _SESSION_H_
#define _SESSION_H_
#include "common.h"
struct session_t {
    // control
	uid_t uid;
	int ctrl_fd;
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];

	// data connect
	struct sockaddr_in * port_addr;
	int pasv_listen_fd;
	int data_fd;
	int data_process;

	// limit control
	unsigned int bw_upload_rate_max;
	unsigned int bw_download_rate_max;
	long bw_transfer_start_sec;
	long bw_transfer_start_usec;

	// pipe of father and child
	int parent_fd;
	int child_fd;

	// ftp 
	int is_ascii;
	long long restart_pos;
	char * rnfr_name;
	int abor_received;

	// connect num limit
	unsigned int num_clients;
	unsigned int num_this_ip;
};


void begin_session(session_t * sess);

#endif // _SESSION_H
