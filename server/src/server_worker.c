#include "server_types.h"
#include "server_worker.h"
#include "conn.h"

#include <assert.h>

static struct smtp_data *smtp_data[MAX_CONN_CNT];
static int64_t smtp_data_cnt;

static struct smtp_data *smtp_data_create(void *conn)
{
	static uint32_t id;
	struct smtp_data *s = (struct smtp_data *)malloc(sizeof(struct smtp_data));
	if (s == NULL)
		return s;

	s->conn = conn;
	s->id = id++;
	//s->state = SMTP_ST_INIT;

	return s;
}

static void smtp_data_destroy(struct smtp_data *s)
{
	assert(s != NULL);
	free(s);
}

void server_worker_start(struct conn *conn)
{
	if (smtp_data_cnt == MAX_CONN_CNT)
		return;

	if (smtp_data_cnt > 0)
		server_worker_close(conn);

	// TODO: need mutex
	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
		if (smtp_data[i] != NULL)
			continue;

		smtp_data_cnt++;
		smtp_data[i] = smtp_data_create(conn);
		slog_d("worker: new client (smtp_data id %d)", (smtp_data[i])->id);
		break;
	}
}

void server_worker_process(struct conn *conn)
{
	struct buf *buf = conn_get_read_buf(conn);
	char *data = buf->data;
	uint32_t len = buf->len;
	struct smtp_data *s = NULL;

	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
		if (smtp_data[i] == NULL)
			continue;

		if ((smtp_data[i])->conn == conn) {
			s = smtp_data[i];
			break;
		}
	}
	assert(s != NULL);

	slog_d("worker: data from client %.*s", len, data);
	conn_read_buf_flush(conn);

	/*
	s->state = smtp_step(s->state, SMTP_EV_CMD, s);
	if (s->state == SMTP_ST_PARSE_CMD) {
		//parse cmd
		slog_i("%s", "worker: parse cmd");

		s->state = smtp_step(s->state, SMTP_EV_CMD_OK, s);
		s->state = smtp_step(s->state, SMTP_EV_CMD_PROCESSED, s);

		conn_read_buf_get_and_flush(conn, &data, &len);

		char tmp_str[] = "hello from server";

		buf_reset(&conn->write);
		conn_append_to_write_buf(conn, tmp_str, sizeof(tmp_str));

		free(data);

		s->state = smtp_step(s->state, SMTP_EV_RESP_SENT, s);

	} else {
		slog_e("%s", "invalid state. flush data");
		conn_read_buf_flush(conn);
	}*/
}

void server_worker_close(struct conn *conn)
{
	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
		if (smtp_data[i] == NULL)
			continue;

		// TODO: need mutex
		if ((smtp_data[i])->conn == conn) {
			slog_d("worker: destroy conn %d", (smtp_data[i])->id);
			smtp_data_destroy(smtp_data[i]);
			smtp_data[i] = NULL;
			smtp_data_cnt--;

			conn_close(conn);
			break;
		}
	}
}

