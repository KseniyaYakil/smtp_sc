#ifndef _SERVER_SESSION_H_
#define _SERVER_SESSION_H_

#include <inttypes.h>
#include <stdint.h>

struct session {
	void *conn;
	uint32_t id;
};


struct session *session_create(void *conn);
void session_destroy(struct session *s);
void session_process_data_from_client(struct session *s, const char *data, uint32_t len);


#endif // _SERVER_SESSION_H_
