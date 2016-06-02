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
#include "privsock.h"


struct ftpcmd_t {
	const char * cmd;
	void (*cmd_handler)(session_t * sess);
};

void ftp_reply(session_t * sess, int status, const char * text);
int list_common(session_t * sess, int detail);


// ftp cmd handler
static void do_user(session_t * sess);
static void do_pass(session_t * sess);
static void do_syst(session_t * sess);
static void do_cwd(session_t * sess);
static void do_pwd(session_t * sess);
static void do_type(session_t * sess);
static void do_port(session_t * sess);
static void do_list(session_t * sess);
static void do_nlist(session_t * sess);
static void do_dup(session_t * sess);
static void do_mkd(session_t * sess);
static void do_rkd(session_t * sess);
static void do_delete(session_t * sess);
static void do_rnfr(session_t * sess);
static void do_rnto(session_t * sess);
static void do_size(session_t * sess);
static void do_pasv(session_t * sess);
static void do_retr(session_t * sess);

static void do_site_chmod(session_t * sess, char * chmod_arg);
static void do_site_umask(session_t * sess, char * umask_arg);
static void do_site(session_t * sess);
int get_port_fd(session_t * sess);
int port_active(session_t * sess);
int pasv_active(session_t * sess);
int get_transfer_fd(session_t * sess);

static ftpcmd_t ctrl_cmds[] = 
{
	{"USER", do_user},
	{"PASS", do_pass},
	{"SYST", do_syst},
	{"CWD", do_cwd},
	{"XCWD", do_cwd},
	{"XCUP", do_dup},
	{"CDUP", do_dup},
	{"PWD", do_pwd},
    {"XPWD", do_pwd},
	{"TYPE", do_type},
	{"PORT", do_port},
	{"LIST", do_list},
	{"NLIST", do_nlist},

	{"MKD", do_mkd},
	{"XMKD", do_mkd},
	{"RMD", do_rkd},
	{"XRMD", do_rkd},
	{"DELE", do_delete},
	{"RNFR", do_rnfr},
	{"RNTO", do_rnto},
	{"SITE", do_site},
	{"SIZE", do_size},
	{"PASV", do_pasv},

	{"RETR", do_retr}
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
		//	printf("sess->cmd:[%s], ctrl_cmds=[%s]\n", sess->cmd, ctrl_cmds[i].cmd);
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

void do_syst(session_t * sess) {
	ftp_reply(sess, FTP_SYSTOK, "UNIX Type:L8");
}

// return the current directory
void do_pwd(session_t * sess) {
	char text[1024] = {0};
	char dir[1024+1] = {0};
	getcwd(dir, 1024);
	sprintf(text, "\"%s\"", dir);
	ftp_reply(sess, FTP_PWDOK, text);
}

// change current directory to arg
void do_cwd(session_t * sess) {
	if (chdir(sess->arg) < 0) {
		ftp_reply(sess, FTP_FILEFAIL, "Failed ti change directory.");
		return ;
	}

	ftp_reply(sess, FTP_CWDOK, "Directory successfully changed.");
}

// the transport file type
void do_type(session_t * sess) {
	if (strcmp(sess->arg, "A") == 0) {
		sess->is_ascii = 1;
		ftp_reply(sess, FTP_TYPEOK, "Switching to ASCII mode.");
	} else if (strcmp(sess->arg, "I") == 0) {
		sess->is_ascii = 0;
		ftp_reply(sess, FTP_TYPEOK, "Switching to Binary mode.");
	} else {
		ftp_reply(sess, FTP_BADCMD, "Unrecognised TYPE command.");
	}
}

void do_port(session_t * sess) {
	printf("do_port");
	unsigned int v[6];
	sscanf(sess->arg, "%u,%u,%u,%u,%u,%u", &v[2], &v[3], &v[4], &v[5], &v[0], &v[1]);
	sess->port_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	sess->port_addr->sin_family = AF_INET;
	unsigned char * p = (unsigned char *)&sess->port_addr->sin_port;
	p[0] = v[0];
	p[1] = v[1];

	p = (unsigned char *)&sess->port_addr->sin_addr;
	p[0] = v[2];
	p[1] = v[3];
	p[2] = v[4];
	p[3] = v[5];

	ftp_reply(sess, FTP_PORTOK, "PORT command successful. Consider using PORT.");
}


void do_pasv(session_t * sess) {
	char ip[16] ={0};
	getlocalip(ip);

	priv_sock_send_cmd(sess->child_fd, PRIV_SOCK_PASV_LISTEN);
	unsigned short port = (int) priv_sock_get_int(sess->child_fd);

	unsigned int v[4];
	sscanf(ip, "%u.%u.%u.%u", &v[0], &v[1], &v[2], &v[3]);
	char text[1024] = {0};
	sprintf(text, "Entering Passive Mode (%u,%u,%u,%u,%u,%u).",
			v[0], v[1], v[2], v[3], port >> 8, port&0xFF);

	printf("do_pasv: %s\n", text);
	ftp_reply(sess, FTP_PASVOK, text);
}


int port_active(session_t * sess) {
	printf("port_active\n");
	if (sess->port_addr) {
		return 1;
	}
	return 0;
}

int pasv_active(session_t * sess) {
	printf("pasv_active\n");
	priv_sock_send_cmd(sess->child_fd, PRIV_SOCK_PASV_ACTIVE);
	int active = priv_sock_get_int(sess->child_fd);
	if (active) {
		if (port_active(sess)) {
			ERR_EXIT("boht port and pasv are active.");
		}
		return 1;
	}
	return 0;
}


// connect to client
int get_port_fd(session_t * sess) {
    // communicate with parent using unix field socket
	printf("get_port_fd\n");
	priv_sock_send_cmd(sess->child_fd, PRIV_SOCK_GET_DATA_SOCK);
	unsigned short port = ntohs(sess->port_addr->sin_port);
	char *ip = inet_ntoa(sess->port_addr->sin_addr);
	priv_sock_send_int(sess->child_fd, (int)port);
	priv_sock_send_buf(sess->child_fd, ip, strlen(ip));

	char res = priv_sock_get_result(sess->child_fd);
	if (res == PRIV_SOCK_RESULT_BAD) {
		return 0;
	} else if (res == PRIV_SOCK_RESULT_OK) {
		sess->data_fd = priv_sock_recv_fd(sess->child_fd);
	}
	return 1;
}

int get_pasv_fd(session_t * sess) {
	printf("get_pasv_fd\n");
	priv_sock_send_cmd(sess->child_fd, PRIV_SOCK_PASV_ACCEPT);
	char res = priv_sock_get_result(sess->child_fd);

	if (res == PRIV_SOCK_RESULT_BAD) {
		return 0;
	} else if (res == PRIV_SOCK_RESULT_OK) {
		sess->data_fd = priv_sock_recv_fd(sess->child_fd);
	}

	return 1;
}

int get_transfer_fd(session_t * sess) {
	printf("get_transfer_fd\n");
	if (port_active(sess) == 0 && pasv_active(sess) == 0) {
		ftp_reply(sess, FTP_BADSENDCONN, "Use PORT or PASV first.");
		return 0;
	}

	printf("port_active\n");
	int ret = 1;
	if (port_active(sess)) {
		if (get_port_fd(sess) == 0) {
			printf("get_port_fd == 0\n");
			ret = 0;
		}
	}

	if (pasv_active(sess)) {
		if (get_pasv_fd(sess) == 0) {
			ret = 0;
		}
	}

	if (sess->port_addr) {
		free(sess->port_addr);
		sess->port_addr = NULL;
	}

	return ret;
}



/**
 *command about file operation
 */
int list_common(session_t * sess, int detail) {
    //printf("list_common\n");
	DIR * dir = opendir(".");
	if (dir == NULL) {
		printf("dir == null\n");
		return 0;
	}

	struct dirent * dt;
	struct stat sbuf;

	while ((dt = readdir(dir)) != NULL) {
	//	printf("name:%s\n", dt->d_name);
		if (lstat(dt->d_name, &sbuf) < 0) {
			continue;
		}
		if (dt->d_name[0] == '.') continue;

		char buf[1024] = {0};
		if (detail) {
			const char * perms = statbuf_get_perms(&sbuf);
			int off = 0;
			off += sprintf(buf, "%s ", perms);
			off += sprintf(buf + off, " %3d %d-8d %-8d ", sbuf.st_nlink, sbuf.st_uid, sbuf.st_gid);
			off += sprintf(buf + off, "%8lu ", (unsigned long)sbuf.st_size);

			const char * datebuf = statbuf_get_date(&sbuf);
			off += sprintf(buf + off, "%s ", datebuf);
			if (S_ISLNK(sbuf.st_mode)) {
				char tmp[1024] = {0};
				readlink(dt->d_name, tmp, sizeof(tmp));
				off += sprintf(buf + off, "%s -> %s\r\n", dt->d_name, tmp);
			} else {
				off += sprintf(buf + off, "%s\r\n", dt->d_name);
			}
		} else {
			sprintf(buf, "%s\r\n", dt->d_name);
		}
      //  printf("sess->data_fd:%d, fileInfor:%s\n", sess->data_fd, buf);
		writen(sess->data_fd, buf, strlen(buf));
	}

	closedir(dir);
	return 1;
}


void do_list(session_t * sess) {
	printf("do_list\n");
	if (get_transfer_fd(sess) == 0) {
		printf("get_transfer_fd == 0");
		return ;
	}

	ftp_reply(sess, FTP_DATACONN, "Here comes the directory listing.");

	list_common(sess, 1);
	close(sess->data_fd);
	sess->data_fd = -1;

	ftp_reply(sess, FTP_TRANSFEROK, "Directory send OK.");
}


void do_dup(session_t * sess) {
	if (chdir("..")	 < 0) {
		ftp_reply(sess, FTP_FILEFAIL, "Failed to change directory.");
		return ;
	}

	ftp_reply(sess, FTP_CWDOK, "Directory successfully changed.");
}

void do_nlist(session_t * sess) {
	printf("do_nlist\n");
	if (get_transfer_fd(sess) == 0) {
		printf("get_transfer_fd == 0");
		return ;
	}

	ftp_reply(sess, FTP_DATACONN, "Here comes the directory listing.");

	list_common(sess, 0);
	close(sess->data_fd);
	sess->data_fd = -1;

	ftp_reply(sess, FTP_TRANSFEROK, "Directory send OK.");
}

void do_mkd(session_t * sess) {
	if (mkdir(sess->arg, 0777) < 0) {
		ftp_reply(sess, FTP_FILEFAIL, "Create directory operation failed.");
		return ;
	}

	char text[4096] = {0};
	if (sess->arg[0] == '/') {
		sprintf(text, "%s created", sess->arg);
	} else {
		char dir[4096+1];
		getcwd(dir, 4096);
		if (dir[strlen(dir)-1] == '/') {
			sprintf(text, "%s%s created", dir, sess->arg);
		} else {
			sprintf(text, "%s/%s created", dir, sess->arg);
		}
	}
	ftp_reply(sess, FTP_MKDIROK, text);
}

void do_rkd(session_t * sess) {
	if (rmdir(sess->arg) < 0) {
		ftp_reply(sess, FTP_FILEFAIL, "Remove directory operation failed.");
		return ;
	}
	ftp_reply(sess, FTP_RMDIROK, "Remove directory operation successful.");
}

void do_delete(session_t * sess) {
	if (unlink(sess->arg) < 0) {
		ftp_reply(sess, FTP_FILEFAIL, "Delete operation failed.");
	}
	ftp_reply(sess, FTP_DELEOK, "Delete operation successful.");
}

void do_rnfr(session_t * sess) {
	sess->rnfr_name = (char*)malloc(strlen(sess->arg)+1);
	memset(sess->rnfr_name, 0, strlen(sess->arg)+1);
	strcpy(sess->rnfr_name, sess->arg);
	ftp_reply(sess, FTP_RNFROK, "Ready for RNTO.");
}

void do_rnto(session_t * sess) {
	if (sess->rnfr_name == NULL) {
		ftp_reply(sess, FTP_NEEDRNFR, "RNFT required first.");
		return ;
	}

	rename(sess->rnfr_name, sess->arg);
	ftp_reply(sess, FTP_RENAMEOK, "Rename successful.");

	free(sess->rnfr_name);
	sess->rnfr_name = NULL;
}

static void do_site_chmod(session_t * sess, char * chmod_arg) {
	if (chmod_arg == NULL) {
		ftp_reply(sess, FTP_BADCMD, "SITE CHMOD needs 2 arguments.");
		return ;
	}

	char perms[100] = {0};
	char file[1024] = {0};
	printf("do_site_chmod:%s\n", chmod_arg);
	str_split(chmod_arg, perms, file, ' ');
	printf("file:%s, mode:%s\n", file, perms);

	if (strlen(file) == 0) {
		ftp_reply(sess, FTP_BADCMD, "SITE CHMOD needs 2 arguments.");
		return ;
	}

	unsigned int mode = str_octal_to_uint(perms);
	printf("mode:%d",mode);
	if (chmod(file, mode) < 0) {
		ftp_reply(sess, FTP_CHMODOK, "SITE CHMOD command failed.");
		return ;
	}

	ftp_reply(sess, FTP_CHMODOK, "SITE CHMOD command ok.");
} 

static void do_site_umask(session_t * sess, char * umask_arg) {
	if (umask_arg == NULL) {
		char text[1024] = {0};
		sprintf(text, "Your current umask is 0%0", tunable_local_umask);
		ftp_reply(sess, FTP_UMASKOK, text);
	} else {
		unsigned int mask = str_octal_to_uint(umask_arg);
		umask(mask);
	    char text[1024] = {0};
		sprintf(text, "Your umask set to 0%0", mask);
		ftp_reply(sess, FTP_UMASKOK, text);
	}
}


void do_site(session_t * sess) {
	// SITE HELP
	// SITE CHMOD
	// SITE UMASK
	char cmd[100] = {0};
	char arg[100] = {0};

	str_split(sess->arg, cmd, arg, ' ');
	if (strcmp(cmd, "CHMOD") == 0) {
		// chmod
		do_site_chmod(sess, arg);
	} else if (strcmp(cmd, "UMASK") == 0) {
		// umask
		do_site_umask(sess, arg);
	} else if (strcmp(cmd, "HELP") == 0) {
		ftp_reply(sess, FTP_SITEHELP, "CHMOD UMASK HELP.");
	} else {
		ftp_reply(sess, FTP_BADCMD, "Unknown SITE command.");
	}
}


void do_size(session_t * sess) {
	struct stat buf;
	if (stat(sess->arg, &buf) < 0) {
		ftp_reply(sess, FTP_FILEFAIL, "SIZE operation failed.");
		return ;
	}

	if (!S_ISREG(buf.st_mode)) {
		ftp_reply(sess, FTP_FILEFAIL, "Could not get file size.");
		return ;
	}

	char text[1024] = {0};
	sprintf(text, "%lld", (long long)buf.st_size);
	ftp_reply(sess, FTP_SIZEOK, text);
}


void do_retr(session_t * sess) {
	printf("do_retr\n");
	if (get_transfer_fd(sess) == 0) return ;

	long long offset = sess->restart_pos;
	sess->restart_pos = 0;

	int fd = open(sess->arg, O_RDONLY);
	if (fd == -1) {
		ftp_reply(sess, FTP_FILEFAIL, "Fail to open file.");
		return ;
	}

	int ret = lock_file_read(fd);
	if (-1 == ret) {
		ftp_reply(sess, FTP_FILEFAIL, "Fail to open file.");
		return ;
	}

	struct stat sbuf;
	ret = fstat(fd, &sbuf);
	if (!S_ISREG(sbuf.st_mode)) {
		ftp_reply(sess, FTP_FILEFAIL, "Fail to open file.");
		return ;
	}

	if (offset != 0) {
		lseek(fd, offset, SEEK_SET);
	}

	char text[1024] = {0};
	if (sess->is_ascii) {
		sprintf(text, "Opening ASCII mode data connection for %s (%lld bytes).", sess->arg, (long long)sbuf.st_size);
	} else {
		sprintf(text, "Opening BINARY mode data connection for %s (%lld bytes).", sess->arg, (long long)sbuf.st_size);
	}

	ftp_reply(sess, FTP_DATACONN, text);

	// start to transmission the file
	long long bytes_to_send = sbuf.st_size;
	if (offset > bytes_to_send) {
		bytes_to_send = 0;
	} else {
		bytes_to_send -= offset;
	}

	char buf[4096];
	int flag = 0;
	while (bytes_to_send > 0) {
		int sendNum = (bytes_to_send <= 4096) ? bytes_to_send : 4096;
		ret = sendfile(sess->data_fd, fd, NULL, sendNum);
		if (ret == -1) {
			flag = 2;
			break;
		}
		bytes_to_send -= ret;
	}

	if (bytes_to_send == 0) flag = 0;

	close(sess->data_fd);
	sess->data_fd = -1;
	close(fd);

	if (flag == 2) {
		ftp_reply(sess, FTP_BADSENDNET, "Failing to write to network stream.");
	} else if (flag == 0) {
		ftp_reply(sess, FTP_TRANSFEROK, "Transfer complete.");
	}
}
