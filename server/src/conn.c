#include "server_types.h"
#include "server_types.h"
#include "conn.h"

#define _GNU_SOURCE
#include <poll.h>

#define MAX_FD_CNT 1024

static int timeout_msec = -1;
static char *hostname_def = "local":

static int hostname_to_ip(const char *hostname, char ip[32])
{
	struct hostent *he = NULL;
	struct in_addr **addr_list = NULL;
	int i = 0;

	if ((he = gethostbyname(hostname)) == NULL) {
		// get the host info
		slog_e("gethostbyname failed for %s (%s)", hostname, strerror(errno));
		return -1;
	}

	addr_list = (struct in_addr **)he->h_addr_list;
	for (i = 0; addr_list[i] != NULL; i++) {
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		slog_i("Address %s was resolved to %s", hostname, ip);
		return 0;
	}

	slog_e("Can't resolve %s to ip", hostname);
	return -1;
}

int run_server()
{
	char ip_addr[32] = "";
	struct sockaddr_in addr;
	int listen_fd;
	uint16_t port = (uint16_t)conf.port;
	struct pollfd fds[MAX_FD_CNT];
	const char *hostname = conf.hostname != NULL ? conf.hostname : hostname_def;

	if (hostname_to_ip(hostname, ip) != 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip_addr);
	addr.sin_port = htons(port);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		slog_e("can't create server socket: %s", strerror(errno));
		return -1;
	}

	int val = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
		slog_e("can't setsockopt: %s", strerror(errno));
		return -1;
	}

	slog_i("ip addr %s", ip_addr);
/*
	if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr)) != 0) {
		slog_e("can't bind on %s:%d: %s", hostname, port, strerror(errno));
		close(sock);
		return -1;
	}
*/
	return 0;
}
