/*************************************************************************
	> File Name: session.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Fri 27 May 2016 07:21:31 AM PDT
 ************************************************************************/

#include "session.h"
#include "ftpproto.h"
#include "privparent.h"

void begin_session(session_t * sess) {
	
	int sockfds[2];
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0) ERR_EXIT("socketpair");

	pid_t pid;
	pid = fork();
	if (pid < 0) {
		ERR_EXIT("fork");
	} else if (pid == 0) {
		close(sockfds[0]);
		sess->child_fd = sockfds[1];
		handle_child(sess);
	} else {
		close(sockfds[1]);
		sess->parent_fd = sockfds[0];
		handle_parent(sess);
	}
}
