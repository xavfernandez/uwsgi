#include "uwsgi.h"

extern struct uwsgi_server uwsgi;



void uwsgi_systemd_notify(char *message) {

	struct msghdr *msghdr = (struct msghdr *) uwsgi.notification_object;
	struct iovec *iovec = msghdr->msg_iov;

	iovec[0].iov_base = "STATUS=";
	iovec[0].iov_len = 7;

	iovec[1].iov_base = message;
	iovec[1].iov_len = strlen(message);

	iovec[2].iov_base = "\n";
        iovec[2].iov_len = 1;

	msghdr->msg_iovlen = 3;

	sendmsg(uwsgi.notification_fd, msghdr, 0);
}

void uwsgi_systemd_notify_ready(void) {

	struct msghdr *msghdr = (struct msghdr *) uwsgi.notification_object;
        struct iovec *iovec = msghdr->msg_iov;

        iovec[0].iov_base = "STATUS=uWSGI is ready\nREADY=1\n";
        iovec[0].iov_len = 30;

	msghdr->msg_iovlen = 1;

	sendmsg(uwsgi.notification_fd, msghdr, 0);
	
}


void uwsgi_systemd_init(char *systemd_socket) {

	struct sockaddr_un *sun;
	struct msghdr *msghdr;

	uwsgi.notification_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (uwsgi.notification_fd < 0) {
		uwsgi_error("socket()");
		return;
	}

	sun = uwsgi_malloc(sizeof(struct sockaddr_un));
	memset(sun, 0, sizeof(struct sockaddr_un));
	sun->sun_family = AF_UNIX;
	strncpy(sun->sun_path, systemd_socket, sizeof(sun->sun_path));
	if (sun->sun_path[0] == '@') sun->sun_path[0] = 0;

	msghdr = uwsgi_malloc(sizeof(struct msghdr));
	memset(msghdr, sizeof(struct msghdr), 0);

	msghdr->msg_iov = uwsgi_malloc(sizeof(struct iovec)*3);
	memset(msghdr->msg_iov, 0, sizeof(struct iovec)*3);

	msghdr->msg_name = sun;
	msghdr->msg_namelen = sizeof(struct sockaddr_un);

	uwsgi.notification_object = msghdr;

	uwsgi.notify = uwsgi_systemd_notify;
	uwsgi.notify_ready = uwsgi_systemd_notify_ready;
	
}
