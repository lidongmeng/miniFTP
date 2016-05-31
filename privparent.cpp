/*************************************************************************
	> File Name: privparent.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Fri 27 May 2016 06:42:31 PM PDT
 ************************************************************************/

#include "privparent.h"
#include "sysutil.h"
#include "tunable.h"
#include "privsock.h"
#include "sysutil.h"

static void privop_pasv_get_data_sock(session_t * sess);
static void privop_pasv_active(session_t * sess);
static void privop_pasv_listen(session_t * sess);
static void privop_pasv_accept(session_t * sess);

void minimize_privilege() {
	struct passwd * pw = getpwnam("nobody");
	if (pw == NULL) ERR_EXIT("getpwnam");

	if (setegid(pw->pw_gid) < 0) ERR_EXIT("setegid");
	if (seteuid(pw->pw_uid) < 0) ERR_EXIT("seteuid");

	// capset
}

void handle_parent(session_t * sess) {
	// set privilege
	minimize_privilege();
	
	char cmd;
	while (1) {
		cmd = priv_sock_get_cmd(sess->parent_fd);
		switch(cmd) {
			case PRIV_SOCK_GET_DATA_SOCK:
				privop_pasv_get_data_sock(sess);
				break;
			case PRIV_SOCK_PASV_ACTIVE:
				privop_pasv_active(sess);
				break;
			case PRIV_SOCK_PASV_LISTEN:
				privop_pasv_listen(sess);
				break;
			case PRIV_SOCK_PASV_ACCEPT:
				privop_pasv_accept(sess);
				break;
		}
	}
}

static void privop_pasv_get_data_sock(session_t * sess) {
	printf("privop_pasv_get_data_sock\n");
	unsigned short port = (unsigned short) priv_sock_get_init(sess->parent_fd);
	char ip[16] = {0};
	priv_sock_recv_buf(sess->parent_fd, ip, sizeof(ip));

	printf("iip:[%s], port:[%d]\n", ip, port);
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	int fd = tcp_client((unsigned short)3330);
	if (fd == -1) {
		priv_sock_send_result(sess->parent_fd, PRIV_SOCK_RESULT_BAD);
		return ;
	}

	printf("local fd: %d\n", fd);
	if (connect_timeout(fd, &addr, tunable_connect_timeout) < 0) {
		close(fd);
		priv_sock_send_result(sess->parent_fd, PRIV_SOCK_RESULT_BAD);
		return  ;
	}

	priv_sock_send_result(sess->parent_fd, PRIV_SOCK_RESULT_OK);
	priv_sock_send_fd(sess->parent_fd, fd);
	close(fd);
}

static void privop_pasv_active(session_t * sess) {

}

static void privop_pasv_listen(session_t * sess) {

}

static void privop_pasv_accept(session_t * sess) {

}
