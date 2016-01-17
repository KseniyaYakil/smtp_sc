#ifndef _SMTP_PROTO_H_
#define _SMTP_PROTO_H_

#include "buf.h"
#include "server_types.h"
#include "smtp-fsm.h"

#define SMTP_RET_MSG_LEN	1024
#define SMTP_CMD_MIN_LEN	4

enum smtp_cmd {	
	SMTP_CMD_EMPTY,
	SMTP_CMD_HELO,
	SMTP_CMD_EHLO,
	SMTP_CMD_MAIL,
	SMTP_CMD_DATA,
	SMTP_CMD_RCPT,
	SMTP_CMD_RSET,
	SMTP_CMD_VRFY,
	SMTP_CMD_QUIT,

	SMTP_CMD_LAST,

};

extern const char *smtp_cmd[SMTP_CMD_LAST];

struct smtp_data {
	te_smtp_state state;
	const char *name;

	struct smtp_msg {
		uint32_t ret;
		char ret_msg[SMTP_RET_MSG_LEN];
		int ret_msg_len;
	} answer;
};

void smtp_data_init(struct smtp_data *s_data, const char *name);
void smtp_data_destroy(struct smtp_data *s_data);
int smtp_data_process(struct smtp_data *s_data, struct buf *msg);

#endif


