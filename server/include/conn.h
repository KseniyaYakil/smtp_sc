
#ifndef _SERVER_CONN_H_
#define _SERVER_CONN_H_

#include "server_types.h"
#include <stdbool.h>

enum conn_status {
	CON_STATUS_EMPTY = 0,
	CON_STATUS_ENABLED = 1,
	CON_STATUS_CLOSING = 2
};

struct conn {
	uint32_t fd_index;
	enum conn_status status;
};

struct conn_msg {
	struct buf buf;
	struct conn *conn;
};

int run_server();
void conn_close(struct conn *conn);

#endif // _SERVER_CONN_H_

