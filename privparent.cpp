/*************************************************************************
	> File Name: privparent.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Fri 27 May 2016 06:42:31 PM PDT
 ************************************************************************/

#include "privparent.h"

void handle_parent(session_t * sess) {
    // change parent to nobody process
	struct passwd * pw = getpwnam("nobody");
	if (pw == NULL) return ;

	// set gid first and uid second
	if (setegid(pw->pw_gid) < 0) ERR_EXIT("setegid");
	if (seteuid(pw->pw_uid) < 0) ERR_EXIT("seteuid");
	
    char cmd;
	while (1) {
		read(sess->parent_fd, &cmd, 1);
		// parse and deal
	}
}
