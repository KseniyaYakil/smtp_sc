#include "buf.h"
#include "conn.h"
#include "server_types.h"
#include "worker.h"

#include <assert.h>

static struct session *session[MAX_CONN_CNT];
static int64_t session_cnt;

static struct session *session_create(void *conn)
{
	static uint32_t id;
	struct session *s = (struct session *)malloc(sizeof(struct session));
	if (s == NULL)
		return s;

	s->conn = conn;
	s->id = id++;
	smtp_data_init(&s->s_data, conf.hostname);

	return s;
}

static void session_destroy(struct session *s)
{
	assert(s != NULL);
	smtp_data_destroy(&s->s_data);
	free(s);
}

static struct session *session_get_by_conn(struct conn *conn)
{
	struct session *s = NULL;

	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
		if (session[i] == NULL)
			continue;

		if ((session[i])->conn == conn) {
			s = session[i];
			break;
		}
	}
	return s;
}

void worker_start(struct conn *conn)
{
	if (session_cnt == MAX_CONN_CNT)
		return;

	if (session_cnt > 0)
		worker_close(conn);

	// TODO: need mutex
	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
		if (session[i] != NULL)
			continue;

		session_cnt++;
		session[i] = session_create(conn);
		slog_d("worker: new client (session id %d)", (session[i])->id);
		break;
	}
}

static int worker_send_answer(struct session *s)
{
	char answer[SMTP_RET_MSG_LEN + 4];
	uint32_t ans_len;
	struct conn *conn = s->conn;
	struct smtp_msg *msg = &s->s_data.answer;

	if ((ans_len = snprintf(answer, sizeof(answer), "%d %.*s",
				msg->ret, msg->ret_msg_len, msg->ret_msg)) < 0) {
		slog_e("%s", strerror(errno));
		abort();
	}

	slog_d("sending answer to client %.*s", ans_len, answer);
	buf_reset(&conn->write);
	conn_append_to_write_buf(conn, answer, ans_len);

	return 0;
}

void worker_process(struct conn *conn)
{
	struct buf client_msg = BUF_STATIC_INITIALIZER();
	struct buf *c_msg = &client_msg;
	struct session *s;

	s = session_get_by_conn(conn);
	assert(s != NULL);

	conn_read_buf_get_and_flush(conn, &c_msg);
	slog_d("worker: data from client %.*s",
		client_msg.len, client_msg.data);

	if (smtp_data_process(&s->s_data, &client_msg) != 0) {
		// internal error -> close connection
		slog_e("internal error occured in session %"PRIu32" close conn", s->id);
		worker_close(s->conn);

		return;
	}

	worker_send_answer(s);


	// TODO: free data

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

void worker_close(struct conn *conn)
{
	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
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

