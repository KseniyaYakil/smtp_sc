#ifndef _SERVER_WORKER_H_
#define _SERVER_WORKER_H_

#include "session.h"
#include "server_types.h"
#include "conn.h"

void server_worker_start(struct conn *conn);
void server_worker_process(struct conn *conn);
void server_worker_close(struct conn *conn);

#endif // _SERVER_WORKER_H_
