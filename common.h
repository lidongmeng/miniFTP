/*************************************************************************
	> File Name: commom.h
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Wed 25 May 2016 06:18:49 AM PDT 
************************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_


#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>

#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <ctype.h>

#include <pwd.h>
#include <shadow.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>

#define ERR_EXIT(m) \
	do { \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)

#define CONF_FILE_NAME "miniftp.conf"


#define MAX_COMMAND_LINE 1024
#define MAX_COMMAND 32
#define MAX_ARG 1024

#endif //_COMMON_H_
