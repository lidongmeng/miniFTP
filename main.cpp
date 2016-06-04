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
#include "hash.h"
#include "ftpproto.h"
#include "ftpcodes.h"
void handle_sigchld(int signo);

static unsigned int s_children;

static hash_t * s_ip_count_hash;
static hash_t * s_pid_ip_hash;

void check_limits(session_t * sess);
unsigned int hash_func(unsigned int buckets, void *key);
unsigned int handle_ip_count(void * ip);
void drop_ip_count(void *ip);


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

	
	struct sockaddr_in clientAddr;
	int connfd;
	pid_t pid;

	session_t sess = {
		0, -1, "", "", "",
		NULL, -1, -1, 0,
		0, 0, 0, 0,
		-1, -1,
		0, 0, NULL, 0,
		0, 0
	};

	sess.bw_download_rate_max = tunable_download_max_rate;
	sess.bw_upload_rate_max = tunable_upload_max_rate;

	s_ip_count_hash = hash_alloc(256, hash_func);
	s_pid_ip_hash = hash_alloc(256, hash_func);
	signal(SIGCHLD, handle_sigchld);
    
	// establish a socket in the server
	// bind it to a port: port can be parse from the configure file
	int listenfd = tcp_server(NULL, tunable_listen_port);
	
	while (1) {
		connfd = accept_timeout(listenfd, &clientAddr, 0);
		if (connfd == -1) ERR_EXIT("accept_timeout");
		printf("receive a connection: %d\n", connfd);
		
		unsigned int ip = clientAddr.sin_addr.s_addr;
		++s_children;
		sess.num_clients = s_children;
		sess.num_this_ip = handle_ip_count(&ip);

		// begin session
		pid = fork();
		if (pid == -1) {
			--s_children;
			ERR_EXIT("fork");
		} else if (pid == 0) { // child
			close(listenfd);
			sess.ctrl_fd = connfd;
			check_limits(&sess);
			signal(SIGCHLD, SIG_IGN);
			begin_session(&sess);
		} else { // father
			add_entry(s_pid_ip_hash, &pid, sizeof(pid), &ip, sizeof(unsigned int));
			close(connfd);
		}
	}
	return 0;
}

void check_limits(session_t * sess) {
	printf("check_limits--num_clients:[%d], sess->num_this_ip:[%d]\n", sess->num_clients, sess->num_this_ip);
	if (tunable_max_clients > 0 && sess->num_clients > tunable_max_clients) {
		ftp_reply(sess, FTP_TOO_MANY_USERS, "There are too many connected users, please try later.");
		exit(EXIT_FAILURE);
	}

	if (tunable_max_per_ip > 0 && sess->num_this_ip > tunable_max_per_ip) {
		ftp_reply(sess, FTP_IP_LIMIT, "There are too many connections from your internet address.");
		exit(EXIT_FAILURE);
	}
}


void handle_sigchld(int signo) {
	pid_t pid;
	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
		--s_children;
		unsigned int * ip = (unsigned int *)lookup_entry(s_pid_ip_hash, &pid, sizeof(pid));

		if (ip == NULL) {
			continue;
		}

		drop_ip_count(ip);
		delete_entry(s_pid_ip_hash, &pid, sizeof(pid));
	}
}

unsigned int hash_func(unsigned int buckets, void * key) {
	unsigned int * number = (unsigned int *) key;
	return (*number) % buckets;
}

unsigned int handle_ip_count(void *ip) {
	unsigned int count;
	unsigned int * p_count = (unsigned int *)lookup_entry(s_ip_count_hash, ip, sizeof(unsigned int));
	if (p_count == NULL) {
		count = 1;
	    add_entry(s_ip_count_hash, ip, sizeof(unsigned int), &count, sizeof(unsigned int));
	} else {
		count = *p_count;
		++count;
		*p_count = count;
	}
	return count;
}

void drop_ip_count(void *ip) {
   unsigned int count;
   unsigned int * p_count = (unsigned int *)lookup_entry(s_ip_count_hash, ip, sizeof(unsigned int));
   if (p_count == NULL) return ;
   count = *p_count;
   --count;
   *p_count = count;
   if (count == 0) delete_entry(s_ip_count_hash, ip, sizeof(unsigned int));
}
