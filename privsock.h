/*************************************************************************
	> File Name: privsock.h
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Mon 30 May 2016 01:16:33 AM PDT
 ************************************************************************/

#ifndef _PRISOCK_H_
#define _PRISOCK_H_
#include "session.h"

#define PRIV_SOCK_GET_DATA_SOCK 1
#define PRIV_SOCK_PASV_ACTIVE 2
#define PRIV_SOCK_PASV_LISTEN 3
#define PRIV_SOCK_PASV_ACCEPT 4

#define PRIV_SOCK_RESULT_OK 1
#define PRIV_SOCK_RESULT_BAD 2

void priv_sock_init(session_t * sess);
void priv_sock_close(session_t * sess);
void priv_sock_set_parent_context(session_t * sess);
void priv_sock_set_child_context(session_t * sess);

void priv_sock_send_cmd(int fd, char cmd);
char priv_sock_get_cmd(int fd);
void priv_sock_send_result(int fd, char res);
char priv_sock_get_result(int fd);


void priv_sock_send_init(int fd, int the_int);
int priv_sock_get_init(int fd);
void priv_sock_send_buf(int fd, const char * buf, unsigned int len);
void priv_sock_recv_buf(int fd, char * buf, unsigned int len);
void priv_sock_send_fd(int sock_fd, int fd);
int priv_sock_recv_fd(int sock_fd);

#endif // _PRISOCK_H_
