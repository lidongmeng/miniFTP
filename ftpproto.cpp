/*************************************************************************
	> File Name: ftpproto.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Fri 27 May 2016 07:39:10 AM PDT
 ************************************************************************/

#include "ftpproto.h"
#include "common.h"
#include "sysutil.h"
#include "str.h"
#include "ftpcodes.h"
#include "tunable.h"

struct ftpcmd_t {
	const char * cmd;
	void (*cmd_handler)(session_t * sess);
};

void ftp_reply(session_t * sess, int status, const char * text);

// ftp cmd handler
static void do_user(session_t * sess);
static void do_pass(session_t * sess);


static ftpcmd_t ctrl_cmds[] = 
{
	{"USER", do_user},
	{"PASS", do_pass}
};

void handle_child(session_t * sess) {
	printf("ctrl_fd: %d\n", sess->ctrl_fd);
	ftp_reply(sess, FTP_GREET, "(minftpd 0.1)");
	while (1) {
		memset(sess->cmdline, 0, sizeof(sess->cmdline));
		memset(sess->cmd, 0, sizeof(sess->cmd));
		memset(sess->arg, 0, sizeof(sess->arg));
		// read command and args from the client
		readline(sess->ctrl_fd, sess->cmdline, MAX_COMMAND_LINE);
		// deal with the ftp cmd
	    printf("%s\n", sess->cmdline);
		// delete \r\n
		str_trim_crlf(sess->cmdline);
		// split the string and get the cmd and arg
		str_split(sess->cmdline, sess->cmd, sess->arg, ' ');
		printf("cmd:%s,arg:%s\n", sess->cmd, sess->arg);
		// change to UPPER case
		str_to_upper(sess->cmd);
	
		// find the cmd handler
		int i = 0;
	    int len = sizeof(ctrl_cmds) / sizeof(ctrl_cmds[0]);
		for (i = 0; i < len; ++i) {
			if (strcmp(sess->cmd, ctrl_cmds[i].cmd) == 0) {
				if (ctrl_cmds[i].cmd_handler != NULL) {
					ctrl_cmds[i].cmd_handler(sess);
				} else {
					ftp_reply(sess, FTP_COMMANDNOTIMPL, "Unimplement command.");
				}
				break;
			}
		}

		if (i == len) {
			ftp_reply(sess, FTP_BADCMD, "Unknow command");
		}
	}
}


void ftp_reply(session_t * sess, int status, const char * text) {
	char buf[1024];
	sprintf(buf, "%d %s\r\n", status, text);
	writen(sess->ctrl_fd, buf, strlen(buf));
}

void do_user(session_t * sess) {
	printf("do_user: %s\n", sess->arg);
	struct passwd * mpasswd = getpwnam(sess->arg);
	if (mpasswd == NULL) {
		printf("NULL of getpwnam");
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return ;
	}
	sess->uid = mpasswd->pw_uid;
	ftp_reply(sess, FTP_GIVEPWORD, "Pease specify the password");
}

void do_pass(session_t * sess) {
	// /tec/passwd
	printf("do_pass: %d\n", sess->uid);
	struct passwd * mpasswd = getpwuid(sess->uid);
	if (mpasswd == NULL) {
		printf("do_pass: NULL of getpwuid");
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return ;
	}
	// shadow 
	printf("pw_name: %s\n", mpasswd->pw_name);
	struct spwd * mspwd = getspnam(mpasswd->pw_name);
	if (mspwd == NULL) {
		printf("do_pass: NULL of getspnam\n");
		ERR_EXIT("do_pass");
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect");
	}
	
	char * encrypted_pass = crypt(sess->arg, mspwd->sp_pwdp);
	if (strcmp(encrypted_pass, mspwd->sp_pwdp) != 0) {
		printf("Pass word error\n");
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return ;
	}

	umask(tunable_local_umask);
	setegid(mpasswd->pw_gid);
	seteuid(mpasswd->pw_uid);
	chdir(mpasswd->pw_dir);
	ftp_reply(sess, FTP_LOGINOK, "login successful.");
}
