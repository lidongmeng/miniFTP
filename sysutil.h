/*************************************************************************
	> File Name: sysutil.h
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Wed 25 May 2016 10:03:43 PM PDT
 ************************************************************************/

#ifndef _SYSUTIL_H_
#define _SYSUTIL_H_

#include "common.h"

int tcp_server(const char * host, unsigned short port);
void activate_nonblock(int fd);
void deactivate_nonblock(int fd);

int read_timeout(int fd, unsigned int wait_seconds);
int write_timeout(int fd, unsigned int wait_seconds);

int accept_timeout(int fd, struct sockaddr_in * addr, unsigned int wait_seconds);
int connect_timeout(int fd, struct sockaddr_in * addr, unsigned int wait_seconds);

ssize_t readn(int fd, void * buf, size_t count);
ssize_t writen(int fd, const void * buf, size_t count);
ssize_t readline(int sock_fd, void * buf, size_t max_len);


#endif // _SYSUTIL_H