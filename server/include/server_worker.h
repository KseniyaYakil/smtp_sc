#ifndef _SERVER_WORKER_H_
#define _SERVER_WORKER_H_

#include "conn.h"
#include "server_types.h"

#include <inttypes.h>
#include <stdint.h>

struct smtp_data {
	void *conn;
	uint32_t id;
};

void server_worker_start(struct conn *conn);
void server_worker_process(struct conn *conn);
void server_worker_close(struct conn *conn);

#endif // _SERVER_WORKER_H_
