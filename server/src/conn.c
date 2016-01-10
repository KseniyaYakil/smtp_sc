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
static struct conn *conns[MAX_CONN_CNT];
static struct pollfd fds[MAX_CLIENTS];

struct conn *conn_new(uint32_t fd_index)
{
	static uint32_t id = 0;
	const uint32_t prealloc = 128;

	struct conn *conn = (struct conn *)malloc(sizeof(struct conn));
	conn->id = id++;
	conn->status = CON_STATUS_ENABLED;
	conn->fd_index = fd_index;

	buf_init(&conn->read, prealloc);
	buf_init(&conn->write, prealloc);

	return conn;
}

struct conn *conn_get_new(uint32_t fd_index)
{
	struct conn *conn = NULL;
	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
		if (conns[i] == NULL) {
			conn = conn_new(fd_index);
			assert(conn != NULL);
			conns[i] = conn;
			slog_d("create new conn %"PRIu32, conn->id);
			break;
		}
	}

	return conn;
}

// XXX: calling function marks conn for closing
// MUST call only once for each conn
void conn_close(struct conn *conn)
{
	assert(conn != NULL);

	slog_d("conn close %"PRIu32" was called", conn->id);
	if (conn->status == CON_STATUS_ENABLED) {
		conn->status = CON_STATUS_CLOSING;
		return;
	}

	slog_e("%s", "conn_close was called twice");
	abort();
}

static void conn_free(struct conn *conn)
{
	slog_d("conn free %"PRIu32" was called", conn->id);

	buf_free(&conn->read);
	buf_free(&conn->write);
	free(conn);
}

void conn_close_all(struct pollfd *fds, uint32_t cnt)
{
	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
		if (conns[i] != NULL &&
		    conns[i]->status == CON_STATUS_CLOSING) {
			uint32_t fd_index = conns[i]->fd_index;
			close(fds[fd_index].fd);

			fds[fd_index] = (struct pollfd) {
				.fd = 0,
				.events = POLLIN,
				.revents = 0
			};

			conn_free(conns[i]);
			conns[i] = NULL;
		}
	}
}

struct conn *conn_get_enabled(uint32_t index)
{
	struct conn *conn = NULL;
	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
		if (conns[i] != NULL &&
		    conns[i]->fd_index == index &&
		    conns[i]->status == CON_STATUS_ENABLED) {
			conn = conns[i];
			break;
		}
	}

	return conn;
}

// XXX: server worker MUST free buf
struct conn *conn_append_to_read_buf(uint32_t index, const char *data, uint32_t len)
{
	struct conn *conn = conn_get_enabled(index);
	if (conn == NULL)
		return NULL;

	buf_append(&(conn->read), data, len);

	return conn;
}

int conn_append_to_write_buf(struct conn *conn, const char *data, uint32_t len)
{
	assert(conn != NULL && conn->status == CON_STATUS_ENABLED);
	buf_append(&(conn->write), data, len);

	fds[conn->fd_index].events |= POLLOUT;

	return 0;
}

// XXX: caller MUST free *data_p mem
int conn_read_buf_get_and_flush(struct conn *conn, char **data_p, uint32_t *len)
{
	buf_copy(&conn->read, data_p, len);
	buf_reset(&conn->read);

	return 0;
}

// XXX: caller MUST free *data_p mem
int conn_write_buf_get_and_flush(struct conn *conn, char **data_p, uint32_t *len)
{
	buf_copy(&conn->write, data_p, len);
	buf_reset(&conn->write);

	return 0;
}

void conn_read_buf_flush(struct conn *conn)
{
	buf_reset(&conn->read);
}

struct buf *conn_get_read_buf(struct conn *conn)
{
	return &conn->read;
}

struct buf *conn_get_write_buf(struct conn *conn)
{
	return &conn->write;
}

static int run_server_loop(int listen_fd)
{
	char buf[MAX_BUF_LEN];

	memset(fds, 0, sizeof(fds));
	memset(conns, 0, sizeof(conns));

	fds[0].fd = listen_fd;
	fds[0].events = POLLIN;
	bool end_server = false;

	while (end_server == false) {
		// free closed conn
		conn_close_all(fds, MAX_CLIENTS);

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
				slog_i("client has gone (%d)", fds[i].fd);

				struct conn *conn = conn_get_enabled(i);
				if (conn != NULL);
					thpool_add_work(thpool, (void*)server_worker_close, conn);

				fds[i].events = 0;
				continue;
			}

			if (fds[i].revents & (POLLERR | POLLNVAL)) {
				slog_e("%s", "err occured in poll");
				return -1;
			}

			if (fds[i].revents & POLLIN) {
				slog_d("read data from %d events %"PRIu32, fds[i].fd, fds[i].events);
				int rc;
				bool close_conn = false;
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

					struct conn *conn = conn_append_to_read_buf(i, buf, rc);
					if (conn != NULL) {
						thpool_add_work(thpool, (void*)server_worker_process, conn);
					} else {
						slog_e("%s", "unable to store data from client: conn closing/doesn't exist");
						close_conn = true;
					}
				} while (false);

				if (close_conn == true) {
					struct conn *conn = conn_get_enabled(i);
					if (conn != NULL)
						thpool_add_work(thpool, (void*)server_worker_close, conn);

					fds[i].events = 0;
				}

				continue;
			}

			if (fds[i].revents & POLLOUT) {
				struct conn *conn = conn_get_enabled(i);
				if (conn == NULL) {
					slog_e("%s", "POLLOUT event but no conn");
					continue;
				}

				slog_d("write data from %d: %.*s", fds[i].fd, conn->write.len, conn->write.data);

				int rc = send(fds[i].fd, conn->write.data, conn->write.len, 0);
				buf_reset(&conn->write);

				if (rc < 0) {
					slog_e("%s", "send() failed");

					thpool_add_work(thpool, (void*)server_worker_close, conn);
				}

				fds[i].events &= ~POLLOUT;
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
