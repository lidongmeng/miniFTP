/*************************************************************************
	> File Name: privsock.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Mon 30 May 2016 06:36:18 AM PDT
 ************************************************************************/

#include "privsock.h"
#include "common.h"
#include "sysutil.h"
#include "tunable.h"

void priv_sock_init(session_t * sess) {
	int sockfds[2];
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0) {
		ERR_EXIT("socketpair");
	}
	sess->parent_fd = sockfds[0];
	sess->child_fd = sockfds[1];
}

void priv_sock_close(session_t * sess) {
	if (sess->parent_fd != -1) {
		close(sess->parent_fd);
		sess->parent_fd = -1;
	}

	if (sess->child_fd != -1) {
		close(sess->child_fd);
		sess->child_fd = -1;
	}
}

void priv_sock_set_parent_context(session_t * sess) {
	if (sess->child_fd != -1) {
		close(sess->child_fd);
		sess->child_fd = -1;
	}
}

void priv_sock_set_child_context(session_t * sess) {
	if (sess->parent_fd != -1) {
		close(sess->parent_fd);
		sess->parent_fd = -1;
	}
}

void priv_sock_send_cmd(int fd, char cmd) {
	int ret;
	printf("priv_sock_send_cmd:%c\n", cmd);
	ret = writen(fd, &cmd, sizeof(cmd));
	if (ret != sizeof(cmd)) {
		ERR_EXIT("priv_sock_send_cmd");
	}
}

char priv_sock_get_cmd(int fd) {
	char res;
	int ret;
	ret = readn(fd, &res, sizeof(res));
	if (0 == ret) {
		printf("FTP process exit");
		exit(EXIT_SUCCESS);
	}

	if (ret != sizeof(res)) {
		ERR_EXIT("priv_sock_get_cmd");
	}
	printf("priv_sock_get_cmd:%c\n", res);
	return res;
}

void priv_sock_send_result(int fd, char res) {
	int ret;
	ret = write(fd, &res, sizeof(res));
	if (ret != sizeof(res)) {
		ERR_EXIT("priv_sock_send_result");
	}
}

char priv_sock_get_result(int fd) {
	int ret;
	char res;
	ret = readn(fd, &res, sizeof(res));
	if (ret != sizeof(res)) {
		ERR_EXIT("priv_sock_get_result");
	}
	return res;
}

void priv_sock_send_int(int fd, int the_int) {
	int ret = writen(fd, &the_int, sizeof(the_int));
	if (ret != sizeof(the_int)) {
		ERR_EXIT("priv_sock_send_init");
	}
}

int priv_sock_get_int(int fd) {
	int the_int;
	int ret;
	ret = readn(fd, &the_int, sizeof(the_int));
	if (ret != sizeof(the_int)) {
		ERR_EXIT("priv_sock_get_int");
	}
	return the_int;
}

void priv_sock_send_buf(int fd, const char * buf, unsigned int len) {
	priv_sock_send_int(fd, (int)len);
	int ret = writen(fd, buf, len);
	if (ret != (int)len) {
		ERR_EXIT("priv_sock_send_buf");
	}
}

void priv_sock_recv_buf(int fd, char * buf, unsigned int len) {
	unsigned int recv_len = (unsigned int)priv_sock_get_int(fd);
	if (recv_len > len) {
		ERR_EXIT("priv_sock_recv_buf");
	}

	int ret = readn(fd, buf, recv_len);
	if (ret != (int)recv_len) {
		ERR_EXIT("priv_sock_recv_buf");
	}
}

void priv_sock_send_fd(int sock_fd, int fd) {
	send_fd(sock_fd, fd);
}

int priv_sock_recv_fd(int sock_fd) {
	return recv_fd(sock_fd);
}

