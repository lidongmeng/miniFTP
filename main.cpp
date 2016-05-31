/*************************************************************************
	> File Name: main.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Wed 25 May 2016 06:16:09 AM PDT
 ************************************************************************/
#include "common.h"
#include "parseconf.h"
#include "tunable.h"
#include "sysutil.h"
#include "session.h"

int main() {
	// the ftp server should be run from the root
	if (getuid() != 0) {
		ERR_EXIT("miniftp: must be started as root\n");
	}
	// load configure file
	parseconf_load_file(CONF_FILE_NAME);

	printf("tunable_pasv_enalbe=%d\n", tunable_pasv_enable);
	printf("tunable_port_enable=%d\n", tunable_port_enable);

	printf("tunable_listen_port=%d\n", tunable_listen_port);
	printf("tunalbe_max_clients=%d\n", tunable_max_clients);
	printf("tunable_max_per_ip=%d\n", tunable_max_per_ip);
	printf("tunable_accept_timeout=%d\n", tunable_accept_timeout);
	printf("tunable_connect_timeout=%d\n", tunable_connect_timeout);
	printf("tunable_idle_session_timeout=%d\n", tunable_idle_session_timeout);
	printf("tunable_data_connection_timeout=%d\n", tunable_data_connection_timeout);
	printf("tunable_local_umask=%d\n", tunable_local_umask);
	printf("tunable_upload_max_rate=%d\n", tunable_upload_max_rate);
	printf("tunalbe_download_max_rate=%d\n", tunable_download_max_rate);
	printf("tunable_listen_address=%s\n", tunable_listen_address);

	// establish a socket in the server
	// bind it to a port: port can be parse from the configure file
	int listenfd = tcp_server(NULL, tunable_listen_port);
	

	struct sockaddr_in clientAddr;
	int connfd;
	pid_t pid;

	session_t sess = {
		-1, -1, "", "", "",
		NULL, -1, -1, -1,
		-1, -1,

		0, NULL
	};


	socklen_t clientLen = sizeof(socklen_t);
	while (1) {
		//connfd = accept_timeout(listenfd, &clientAddr, 0);
		connfd = accept(listenfd,(struct sockaddr*) &clientAddr, &clientLen);
		if (connfd == -1) ERR_EXIT("accept_timeout");
		printf("receive a connection: %d\n", connfd);
		// begin session
		pid = fork();
		if (pid == -1) {
			ERR_EXIT("fork");
		} else if (pid == 0) { // child
			close(listenfd);
			sess.ctrl_fd = connfd;
			begin_session(&sess);
		} else { // father
			close(connfd);
		}
	}
	return 0;
}
