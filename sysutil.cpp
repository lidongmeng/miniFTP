/*************************************************************************
	> File Name: sysutil.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Wed 25 May 2016 10:08:51 PM PDT
 ************************************************************************/

#include "sysutil.h"

void activate_nonblock(int fd) {
	int ret;
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1) {
		ERR_EXIT("fcntl");
	}

	flags |= O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flags);
	if (ret == -1) ERR_EXIT("fcntl");
}

void deactivate_nonblock(int fd) {
	int ret;
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1) ERR_EXIT("fcntl");

	flags &= ~O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flags);
	if (ret == -1) ERR_EXIT("fcntl");
}


int tcp_client(unsigned short port) {
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		ERR_EXIT("tcp_client");
	}

	if (port > 0) {
		int on = 1;
		if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on))) < 0) {
			ERR_EXIT("setsockopt");
		}

		char ip[16] = {0};
		getlocalip(ip);
		struct sockaddr_in localaddr;
		memset(&localaddr, 0, sizeof(localaddr));
		localaddr.sin_family = AF_INET;
		localaddr.sin_port = htons(port);
		localaddr.sin_addr.s_addr = inet_addr(ip);
		if (bind(sock, (struct sockaddr*)&localaddr, sizeof(localaddr)) < 0) {
			ERR_EXIT("bind");
		}
	}
	return sock;
}


// use the specific host and port to build the server
// establish a socket and bind socket to the port and ip
// return the sockfd if succeed or -1 if failed
int tcp_server(const char * host, unsigned short port) {
    // establish a socket
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) ERR_EXIT("socket");
		
	// fill the sockaddr
	struct sockaddr_in serverAddr;
	// initilization
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	if (host != NULL) {
         // judge host: hostname or ip
		 if (inet_aton(host, &serverAddr.sin_addr) == 0) { // 0 stands for host not by ip
			struct hostent * hp;
			hp = gethostbyname(host);
			if (hp == NULL) ERR_EXIT("gethostbyname"); // by host name
			serverAddr.sin_addr = *(struct in_addr*)hp->h_addr;
		 }
	} else {
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // network byte sequence --l
	}
	serverAddr.sin_port = htons(port);

    // reuse the sockfd
    int on = 1;
	if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on))) < 0) ERR_EXIT("setsockopt");
	// bind
	if (bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) ERR_EXIT("bind");
	// listen
	if (listen(listenfd, 1024) < 0) ERR_EXIT("listen");

	return listenfd;
}

int getlocalip(char * ip) {
	int sockfd;
	if (-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0))) ERR_EXIT("socket");
	struct ifreq req;
	struct sockaddr_in * host;
	bzero(&req, sizeof(struct ifreq));
	strcpy(req.ifr_name, "ens33");
	if (ioctl(sockfd, SIOCGIFADDR, &req) == -1) ERR_EXIT("ioctl");
	host = (struct sockaddr_in*)&req.ifr_addr;
	strcpy(ip, inet_ntoa(host->sin_addr));
	close(sockfd);
	return 1;
}

int accept_timeout(int fd, struct sockaddr_in * addr, unsigned int wait_seconds) {
	int ret;
	socklen_t addrlen = sizeof(socklen_t);

	if (wait_seconds > 0) {
		fd_set accept_fdset;
		struct timeval timeout;
		FD_ZERO(&accept_fdset);
		FD_SET(fd, &accept_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do {
			ret = select(fd + 1, &accept_fdset, NULL, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);

		if (ret == -1) {
			return -1;
		} else if (ret == 0) {
			errno = ETIMEDOUT;
				return -1;
		}
	}

	if (addr != NULL) {
		ret = accept(fd, (struct sockaddr*)&addr, &addrlen);
	} else {
		ret = accept(fd, NULL, NULL);
	}
	return ret;
}

int connect_timeout(int fd, struct sockaddr_in * addr, unsigned int wait_seconds) {
	int ret;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (wait_seconds > 0) activate_nonblock(fd);

	ret = connect(fd, (struct sockaddr*)addr, addrlen);

	if (ret < 0 && errno == EINPROGRESS) {
		fd_set connect_fdset;
		struct timeval timeout;
		FD_ZERO(&connect_fdset);
		FD_SET(fd, &connect_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		do {
			ret = select(fd + 1, NULL, &connect_fdset, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);

		if (ret == 0) {
			ret = -1;
			errno = ETIMEDOUT;
		} else if (ret < 0) {
			return -1;
		} else if (ret == 1) {
			int err;
			socklen_t socklen = sizeof(err);
			int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
			if (sockoptret == -1) {
				return -1;
			}
			if (err == 0) {
				ret = 0;
			} else {
				errno = err;
				ret = -1;
			}
		}
	}

	if (wait_seconds > 0) {
		deactivate_nonblock(fd);
	}
	return ret;
}


ssize_t readn(int fd, void * buf, size_t count) {
	size_t nleft = count;
	size_t nread;
	char * p = (char*)buf;

	while (nleft > 0) {
		if ((nread = read(fd, buf, nleft)) < 0) {
			if (errno == EINTR) continue;
			else return -1;
		} else if (nread == 0) {
			return count - nleft;
		} else {
			p += nread;
			nleft -= nread;
		}
	}
	return count;
}

ssize_t recv_peek(int fd, void * buf, size_t len) {
	while (1) {
		int ret = recv(fd, buf, len, MSG_PEEK);
		if (ret == -1 && errno == EINTR) continue;
		else return ret;
	}
}

ssize_t writen(int fd, const void * buf, size_t count) {
	//printf("writen:%s", buf);
	size_t nleft = count;
	char * p = (char*)buf;
	size_t nwritten;

	while (nleft > 0) {
		if ((nwritten = write(fd, p, nleft)) < 0) {
			if (errno == EINTR) continue;
			else return -1;
		} else if (nwritten == 0) {
			continue;
		} else {
			nleft -= nwritten;
			p += nwritten;
		}
	}
	return count;
}

ssize_t readline(int sock_fd, void * buf, size_t max_len) {
	int ret;
	int nread;
	char * p = (char*)buf;
	int nleft = max_len;


	while (1) {
		ret = recv_peek(sock_fd, p, nleft);
		if (ret < 0) return ret;
		else if (ret == 0) return ret;

		nread = ret;
		for (int i = 0; i < nread; ++i) {
			if (p[i] == '\n') {
				ret = readn(sock_fd, p, i+1);
				if (ret != i+1) exit(EXIT_FAILURE);
				return ret;
			}
		}

		if (nread > nleft) exit(EXIT_FAILURE);
		nleft -= nread;

		ret = readn(sock_fd, p, nread);
		if (ret != nread) exit(EXIT_FAILURE);

		p += nread;
	}

	return -1;
}

void send_fd(int sock_fd, int fd)
{
    int ret;
    struct msghdr msg;
    struct cmsghdr *p_cmsg;
    struct iovec vec;
    char cmsgbuf[CMSG_SPACE(sizeof(fd))];
    int *p_fds;
    char sendchar = 0;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    p_cmsg = CMSG_FIRSTHDR(&msg);
    p_cmsg->cmsg_level = SOL_SOCKET;
    p_cmsg->cmsg_type = SCM_RIGHTS;
    p_cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
    p_fds = (int*)CMSG_DATA(p_cmsg);
    *p_fds = fd;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;

    vec.iov_base = &sendchar;
    vec.iov_len = sizeof(sendchar);
    ret = sendmsg(sock_fd, &msg, 0);
    if (ret != 1)
        ERR_EXIT("sendmsg");
}

int recv_fd(const int sock_fd)
{
    int ret;
    struct msghdr msg;
    char recvchar;
    struct iovec vec;
    int recv_fd;
    char cmsgbuf[CMSG_SPACE(sizeof(recv_fd))];
    struct cmsghdr *p_cmsg;
    int *p_fd;
    vec.iov_base = &recvchar;
    vec.iov_len = sizeof(recvchar);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    msg.msg_flags = 0;

    p_fd = (int*)CMSG_DATA(CMSG_FIRSTHDR(&msg));
    *p_fd = -1;
    ret = recvmsg(sock_fd, &msg, 0);
    if (ret != 1)
        ERR_EXIT("recvmsg");

    p_cmsg = CMSG_FIRSTHDR(&msg);
    if (p_cmsg == NULL)
        ERR_EXIT("no passed fd");


    p_fd = (int*)CMSG_DATA(p_cmsg);
    recv_fd = *p_fd;
    if (recv_fd == -1)
        ERR_EXIT("no passed fd");

    return recv_fd;
}

const char* statbuf_get_perms(struct stat *sbuf)
{
    static char perms[] = "----------";
    perms[0] = '?';

    mode_t mode = sbuf->st_mode;
    switch (mode & S_IFMT)
    {
    case S_IFREG:
        perms[0] = '-';
        break;
    case S_IFDIR:
        perms[0] = 'd';
        break;
    case S_IFLNK:
        perms[0] = 'l';
        break;
    case S_IFIFO:
        perms[0] = 'p';
        break;
    case S_IFSOCK:
        perms[0] = 's';
        break;
    case S_IFCHR:
        perms[0] = 'c';
        break;
    case S_IFBLK:
        perms[0] = 'b';
        break;
    }

    if (mode & S_IRUSR)
    {
        perms[1] = 'r';
    }
    if (mode & S_IWUSR)
    {
        perms[2] = 'w';
    }
    if (mode & S_IXUSR)
    {
        perms[3] = 'x';
    }
    if (mode & S_IRGRP)
    {
        perms[4] = 'r';
    }
    if (mode & S_IWGRP)
    {
        perms[5] = 'w';
    }
    if (mode & S_IXGRP)
    {
        perms[6] = 'x';
    }
    if (mode & S_IROTH)
    {
        perms[7] = 'r';
    }
    if (mode & S_IWOTH)
    {
        perms[8] = 'w';
    }
    if (mode & S_IXOTH)
    {
        perms[9] = 'x';
    }
    if (mode & S_ISUID)
    {
        perms[3] = (perms[3] == 'x') ? 's' : 'S';
    }
    if (mode & S_ISGID)
    {
        perms[6] = (perms[6] == 'x') ? 's' : 'S';
    }
    if (mode & S_ISVTX)
    {
        perms[9] = (perms[9] == 'x') ? 't' : 'T';
    }

    return perms;
}

const char* statbuf_get_date(struct stat *sbuf)
{
    static char datebuf[64] = {0};
    const char *p_date_format = "%b %e %H:%M";
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t local_time = tv.tv_sec;
    if (sbuf->st_mtime > local_time || (local_time - sbuf->st_mtime) > 60*60*24*182)
    {
        p_date_format = "%b %e  %Y";
    }

    struct tm* p_tm = localtime(&local_time);
    strftime(datebuf, sizeof(datebuf), p_date_format, p_tm);

    return datebuf;
}

static int lock_internal(int fd, int lock_type) {
	int ret;
	struct flock the_lock;
	memset(&the_lock, 0, sizeof(the_lock));
	the_lock.l_type = lock_type;
	the_lock.l_whence = SEEK_SET;
	the_lock.l_start = 0;
	the_lock.l_len = 0;

	do {
		ret = fcntl(fd, F_SETLKW, &the_lock); 
	} while (ret < 0 && errno == EINTR);

	return ret;
}

int lock_file_read(int fd) {
	return lock_internal(fd, F_RDLCK);
}

int lock_file_wrtie(int fd) {
	return lock_internal(fd, F_WRLCK);
}

int unlock_file(int fd) {
	int ret;
	struct flock the_lock;
	memset(&the_lock, 0, sizeof(the_lock));
	the_lock.l_type = F_UNLCK;
	the_lock.l_whence = SEEK_SET;
	the_lock.l_start = 0;
	the_lock.l_len = 0;

    ret = fcntl(fd, F_SETLK, &the_lock); 

	return ret;

}
