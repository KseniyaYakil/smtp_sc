#include "conn.h"
#include "server_types.h"
#include "server_worker.h"
#include "session.h"
#include "thpool.h"

#include <arpa/inet.h>
#include <assert.h>
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
static threadpool thpool;
// TODO: make extendable
static struct conn *conns[MAX_CLIENTS * 2];

struct conn *conn_get_new(uint32_t fd_index)
{
	struct conn *conn = NULL;
	for (uint32_t i = 0; i < MAX_CLIENTS; i++) {
		if (conns[i] == NULL) {
			conn = (struct conn *)malloc(sizeof(struct conn));

			conns[i] = conn;
			conn->status = CON_STATUS_ENABLED;
			conn->fd_index = fd_index;
			break;
		}
	}

	return conn;
}

void conn_close(struct conn *conn)
{
	bool found = false;
	assert(conn != NULL);

	for (uint32_t i = 0; i < MAX_CLIENTS; i++) {
		if (conns[i] == conn) {
			conns[i] = NULL;
			found = true;
			break;
		}
	}

	assert(found == true);
	free(conn);

	//TODO: mark fd[fd_index] as freed ? when err occured in server_worker
	//and need to close conn
}

struct conn *conn_get_enabled(uint32_t index)
{
	struct conn *conn = NULL;
	for (uint32_t i = 0; i < MAX_CLIENTS; i++) {
		if (conns[i] != NULL &&
		    conns[i]->fd_index == index &&
		    conns[i]->status == CON_STATUS_ENABLED) {
			conn = conns[i];
			break;
		}
	}

	return conn;
}

struct conn_msg *conn_form_msg(uint32_t index, const char *data, uint32_t len)
{
	struct conn_msg *msg = (struct conn_msg *)malloc(sizeof(struct conn_msg));

	buf_init(&(msg->buf), len);
	// TODO: process err (when unable to alloc mem)
	buf_append(&(msg->buf), data, len);
	msg->conn = conn_get_enabled(index);

	return msg;
}

static int run_server_loop(int listen_fd)
{
	struct pollfd fds[MAX_CLIENTS];
	char buf[MAX_BUF_LEN];

	memset(fds, 0, sizeof(fds));
	memset(conns, 0, sizeof(conns));

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

						struct conn *conn = conn_get_new(f);
						assert(conn != NULL);

						slog_d("%s", "server: add work to worker on new client");

						thpool_add_work(thpool, (void*)server_worker_start, conn);
						break;
					}
				}
				continue;
			}

			if (fds[i].revents & POLLHUP) {
				slog_d("client has gone (%d)", fds[i].fd);
				//TODO: need to close prev socket?
				struct conn *conn = conn_get_enabled(i);
				assert(conn != NULL);
				thpool_add_work(thpool, (void*)server_worker_close, conn);

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

					struct conn_msg *msg = conn_form_msg(i, buf, rc);
					if (msg == NULL) {
						slog_e("%s", "unable to form conn msg from client");
						abort();
					}

					thpool_add_work(thpool, (void*)server_worker_process, msg);

					/*
					rc = send(fds[i].fd, buf, rc, 0);
					if (rc < 0) {
						slog_e("%s", "send() failed");

						thpool_add_work(thpool, (void*)server_worker_close, cl);
						close_conn = true;
						break;
					}
					*/
				} while (true);

				if (close_conn == true) {
					struct conn *conn = conn_get_enabled(i);
					assert(conn != NULL);
					thpool_add_work(thpool, (void*)server_worker_close, conn);

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

	// create thread pool
	thpool = thpool_init(conf.thread_cnt);

	int res;
	if ((res = run_server_loop(sock)) != 0) {
		slog_e("%s", "aborting server due to error");
	}

	thpool_destroy(thpool);

	return res;
}
