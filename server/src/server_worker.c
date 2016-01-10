#include "server_types.h"
#include "server_worker.h"
#include "conn.h"

#include <assert.h>

static struct session *session[MAX_CLIENTS];
static int64_t session_cnt;

void server_worker_start(struct conn *conn)
{
	if (session_cnt == MAX_CLIENTS)
		return;

	if (session_cnt > 0)
		server_worker_close(conn);

	// TODO: need mutex
	for (uint32_t i = 0; i < MAX_CLIENTS; i++) {
		if (session[i] != NULL)
			continue;

		session_cnt++;
		session[i] = session_create(conn);
		slog_d("worker: new client (session id %d)", (session[i])->id);
		break;
	}
}

void server_worker_process(struct conn *conn)
{
	struct buf *buf = conn_get_read_buf(conn);

	slog_d("worker: data from client %.*s", buf_get_len(buf), buf_get_data(buf));

	char *data;
	uint32_t len;
	conn_read_buf_get_and_flush(conn, &data, &len);

	//TODO: add send here
	char tmp_str[] = "hello from server";

	buf_reset(&conn->write);
	conn_append_to_write_buf(conn, tmp_str, sizeof(tmp_str));

	free(data);
}

void server_worker_close(struct conn *conn)
{
	for (uint32_t i = 0; i < MAX_CLIENTS; i++) {
		if (session[i] == NULL)
			continue;

		// TODO: need mutex
		if ((session[i])->conn == conn) {
			slog_d("worker: destroy conn %d", (session[i])->id);
			session_destroy(session[i]);
			session[i] = NULL;
			session_cnt--;

			conn_close(conn);
			break;
		}
	}
}

