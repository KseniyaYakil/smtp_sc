#ifndef _SERVER_WORKER_H_
#define _SERVER_WORKER_H_

#include "conn.h"
#include "server_types.h"
#include "smtp_proto.h"

#include <inttypes.h>
#include <stdint.h>

struct session {
	void *conn;
	uint32_t id;
	struct smtp_data s_data;
};

void worker_start(struct conn *conn);
void worker_process(struct conn *conn);
void worker_close(struct conn *conn);

#endif // _SERVER_WORKER_H_
