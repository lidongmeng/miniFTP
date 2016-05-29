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
	// pipe of father and child
	int parent_fd;
	int child_fd;

};


void begin_session(session_t * sess);

#endif // _SESSION_H
