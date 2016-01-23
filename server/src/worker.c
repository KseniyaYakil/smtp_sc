#include "buf.h"
#include "conn.h"
#include "server_types.h"
#include "worker.h"

#include <pcre.h>
#include <assert.h>

static struct session *session[MAX_CONN_CNT];
static int64_t session_cnt;

static pcre *eof_line;

__attribute__((constructor))
static void worker_internal_init(void)
{
	const char *err;
	int err_off;
	char *str_eof_line = "^((?:.+?"CRLF")|("CRLF"))";

	eof_line = pcre_compile(str_eof_line, PCRE_CASELESS, &err, &err_off, NULL);
	if (eof_line == NULL) {
		slog_e("%s", "incorrect regular expression for `eof_line'");
		abort();
	}
}

static struct session *session_create(void *conn)
{
	static uint32_t id;
	struct session *s = (struct session *)malloc(sizeof(struct session));
	if (s == NULL)
		return s;

	s->conn = conn;
	s->id = id++;
	smtp_data_init(&s->s_data, conf.hostname, conf.mail_dir,
			conf.queue_dir, ((struct conn *)conn)->ip);

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

static int worker_send_answer(struct session *s)
{
	char answer[SMTP_RET_MSG_LEN + 4];
	uint32_t ans_len;
	struct conn *conn = s->conn;
	struct smtp_msg *msg = &s->s_data.answer;

	if (msg->ret_msg_len == 0)
		return 0;

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

void worker_start(struct conn *conn)
{
	if (session_cnt == MAX_CONN_CNT)
		return;

	if (session_cnt > 0)
		worker_close(conn);

	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
		if (session[i] != NULL)
			continue;

		session_cnt++;
		session[i] = session_create(conn);

		worker_send_answer(session[i]);
		slog_d("worker: new client (session id %d)", (session[i])->id);
		break;
	}
}

static void pcre_match_line(const char *data, int len,
			    const char **match_off, int *match_len)
{
	int ovec[24];
	int ovecsize = sizeof(ovec);
	int off;
	int rc = pcre_exec(eof_line, 0, data, len, 0, 0, ovec, ovecsize);

	*match_len = 0;
	*match_off = NULL;

	if (rc < 0)
		return;

	off = ovec[2];
	*match_len = ovec[3] - ovec[2];
	*match_off = data + off;
}

static void worker_get_line(struct conn *conn, struct buf *msg)
{
	struct buf *read_buf = conn_read_buf_get(conn);
	const char *line;
	int line_len;

	pcre_match_line(buf_get_data(read_buf), buf_get_len(read_buf),
			&line, &line_len);

	buf_reset(msg);
	if (line_len == 0)
		return;

	buf_append(msg, line, line_len);

	buf_move(read_buf, line_len);
}

void worker_process(struct conn *conn)
{
	struct buf client_msg __attribute__((__cleanup__(buf_free))) = BUF_STATIC_INITIALIZER();
	struct session *s;

	s = session_get_by_conn(conn);
	assert(s != NULL);

	worker_get_line(conn, &client_msg);

	if (buf_get_len(&client_msg) == 0)
		return;

	int ret = smtp_data_process(&s->s_data, &client_msg);

	if (ret != 0) {
		if (ret == -1) {
			slog_e("internal error occured in"
			       " session %"PRIu32" close conn", s->id);
		} else {
			slog_d("QUIT smtp auto: close conn %"PRIu32, s->id);
			worker_send_answer(s);
		}

		worker_close(s->conn);

		return;
	}

	worker_send_answer(s);

	if (conn->read.len != 0)
		worker_process(conn);
}

void worker_close(struct conn *conn)
{
	for (uint32_t i = 0; i < MAX_CONN_CNT; i++) {
		if (session[i] == NULL)
			continue;

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

