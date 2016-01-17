#ifndef _SMTP_PROTO_H_
#define _SMTP_PROTO_H_

#include "buf.h"
#include "server_types.h"
#include "smtp-fsm.h"

#include <pcre.h>

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

struct smtp_data {
	te_smtp_state state;
	const char *name;
	enum smtp_cmd cur_cmd;

	struct smtp_client {
		const char *data; // from client
		int len;
		char *domain;
	} client;

	struct smtp_msg {
		uint32_t ret;
		char ret_msg[SMTP_RET_MSG_LEN];
		int ret_msg_len;
	} answer;
};

#define SMTP_DATA_FORM_ANSWER(s_, ret_, msg_) {					\
	(s_)->answer.ret = ret_;						\
	(s_)->answer.ret_msg_len = snprintf((s_)->answer.ret_msg,		\
					     sizeof((s_)->answer.ret_msg),	\
					     "%s\r\n", msg_);			\
	if ((s_)->answer.ret_msg_len < 0) {					\
		slog_e("err in snprintf %s", strerror(errno));			\
		abort();							\
	}									\
}

struct smtp_cmd_info {
	char *cmd;
	uint8_t cmd_len;
	te_smtp_event evt;

	struct smtp_reg {
		pcre *re;
		const char *str;
	} re;
};

extern struct smtp_cmd_info smtp_cmd_arr[SMTP_CMD_LAST];

void smtp_data_init(struct smtp_data *s_data, const char *name);
void smtp_data_destroy(struct smtp_data *s_data);
int smtp_data_process(struct smtp_data *s_data, struct buf *msg);
void smtp_data_reset(struct smtp_data *s_data);

#endif


