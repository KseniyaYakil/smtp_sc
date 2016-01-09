#include "server_types.h"
#include "session.h"

#include <assert.h>

struct session *session_create(void *conn)
{
	static uint32_t id;
	struct session *s = (struct session *)malloc(sizeof(struct session));
	if (s == NULL)
		return s;

	s->conn = conn;
	s->id = id++;

	return s;
}

void session_destroy(struct session *s)
{
	assert(s != NULL);
	free(s);
}

void session_process_data_from_client(struct session *s, const char *data, uint32_t len)
{
	slog_i("session: id=%"PRIu32", recv data from client %.*s", s->id, len, data);
}
