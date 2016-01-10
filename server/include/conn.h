
#ifndef _SERVER_CONN_H_
#define _SERVER_CONN_H_

#include "server_types.h"
#include <stdbool.h>

#define MAX_CONN_CNT	MAX_CLIENTS * 2

enum conn_status {
	CON_STATUS_EMPTY = 0,
	CON_STATUS_ENABLED = 1,
	CON_STATUS_CLOSING = 2
};

struct conn {
	uint32_t id;
	uint32_t fd_index;
	enum conn_status status;

	struct buf read; // read data from client
	struct buf write; // write data to client
};

int run_server();
int conn_append_to_write_buf(struct conn *conn, const char *data, uint32_t len);
int conn_read_buf_get_and_flush(struct conn *conn, char **data_p, uint32_t *len);
struct buf *conn_get_read_buf(struct conn *conn);
void conn_read_buf_flush(struct conn *conn);
struct buf *conn_get_write_buf(struct conn *conn);
void conn_close(struct conn *conn);

#endif // _SERVER_CONN_H_

