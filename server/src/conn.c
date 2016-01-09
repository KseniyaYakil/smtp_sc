#include "conn.h"
#include "server_types.h"
#include "session.h"

#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#define _GNU_SOURCE
#include <poll.h>
#include <unistd.h>

#define MAX_FD_CNT	1024
#define MAX_Q_LEN	10
#define MAX_BUF_LEN	1024

static int timeout_msec = -1;
static char ip_def[32] = "127.0.0.1";

static int run_server_loop(int listen_fd)
{
	struct pollfd fds[MAX_FD_CNT];
	struct session *all_sessions[MAX_FD_CNT];
	char buf[MAX_BUF_LEN];

	memset(fds, 0, sizeof(fds));
	memset(all_sessions, 0, sizeof(all_sessions));

	fds[0].fd = listen_fd;
	fds[0].events = POLLIN;
	bool end_server = false;

	while (end_server == false) {
		int nevents = poll(fds, MAX_FD_CNT, timeout_msec);
		if (nevents < 0) {
			slog_e("err in poll %s", strerror(errno));
			if (errno != EAGAIN)
				return -1;
			continue;
		}

		for (uint32_t i = 0; i < MAX_FD_CNT && nevents > 0; i++) {
			if (fds[i].revents == 0)
				continue;

			nevents--;

			if (fds[i].fd == listen_fd) {
				int new_fd = 0;
				while (new_fd != -1) {
					new_fd = accept(listen_fd, NULL, NULL);
					if (new_fd < 0) {
						if (errno != EWOULDBLOCK) {
							slog_e("accept() failed: %s", strerror(errno));
							end_server = true;
						}
						break;
					}

					slog_i("new client %d", new_fd);
					for (uint32_t f = 1; f < MAX_FD_CNT; f++) {
						if (fds[f].fd != 0)
							continue;

						fds[f] = (struct pollfd) {
							.fd = new_fd,
							.events = POLLIN,
							.revents = 0
						};
						all_sessions[f] = session_create(&fds[f]);
						break;
					}
				}
				continue;
			}

			if (fds[i].revents & POLLHUP) {
				slog_d("client has gone (%d)", fds[i].fd);
				//TODO: need to close prev socket?
				session_destroy(all_sessions[i]);
				all_sessions[i] = NULL;

				fds[i] = (struct pollfd) {
					.fd = 0,
					.events = POLLIN,
					.revents = 0
				};

				continue;
			}

			if (fds[i].revents & (POLLERR | POLLNVAL)) {
				slog_e("%s", "err occured in poll");
				return -1;
			}

			if (fds[i].revents & POLLIN) {
				slog_i("read data from %d", fds[i].fd);
				int rc;
				bool close_conn;
				do {
					rc = recv(fds[i].fd, buf, sizeof(buf), 0);
					if (rc < 0) {
						if (errno != EWOULDBLOCK) {
							slog_e("%s", "recv() failed");
							close_conn = true;
						}
						break;
					}

					if (rc == 0) {
						slog_d("%s", "  Connection closed\n");
						close_conn = true;
						break;
					}

					slog_d("  %d bytes received\n", rc);

					session_process_data_from_client(all_sessions[i], buf, rc);

					rc = send(fds[i].fd, buf, rc, 0);
					if (rc < 0) {
						slog_e("%s", "send() failed");
						close_conn = true;
						break;
					}
				} while (true);

				// TODO: close session too
				if (close_conn == true) {
					session_destroy(all_sessions[i]);
					all_sessions[i] = NULL;

					close(fds[i].fd);
					memset(&fds[i], 0, sizeof(fds[i]));
				}

				continue;
			}

			if (fds[i].revents & POLLOUT) {
				slog_i("write data from %d", fds[i].fd);
			}
		}

	}

	return 0;
}

int run_server()
{
	char *ip_addr = ip_def;
	struct sockaddr_in addr;
	uint16_t port = (uint16_t)conf.port;

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

	if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr)) != 0) {
		slog_e("can't bind on %s:%d: %s", ip_addr, port, strerror(errno));
		close(sock);
		return -1;
	}

	if (listen(sock, MAX_Q_LEN) != 0) {
		slog_e("unable to listen socket %s", strerror(errno));
		close(sock);
		return -1;
	}

	int status = fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
	if (status == -1) {
		slog_e("can't make server socket non-block: %s", strerror(errno));
		close(sock);
		return -1;
	}

	slog_i("start listening on %s:%d", ip_addr, port);

	return run_server_loop(sock);
}
