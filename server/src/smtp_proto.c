#include "smtp_proto.h"

#include <assert.h>
#include <pcre.h>

#define NUMBER		"(?:\\d+)"
#define SNUM		"(?:\\d{1,3})"
#define DOTNUM		"(?:"SNUM"\\."SNUM"\\."SNUM"\\."SNUM")"
#define NAME		"(?:[a-zA-Z][a-zA-Z\\d\\-]*[a-zA-Z\\d])"
#define ELEMENT		"(?:"NAME"|(?:#"NUMBER")|(?:\\["DOTNUM"\\]))"
#define DOMAIN		"(?:"ELEMENT"(?:\\."ELEMENT")*)"
#define SP		" "
#define CRLF		"\r\n"

// TODO: SP + CRLF
struct smtp_cmd_info smtp_cmd_arr[SMTP_CMD_LAST] = {
	[SMTP_CMD_HELO] = {
		.cmd = "HELO",
		.cmd_len = sizeof("HELO") - 1,
		.evt = SMTP_EV_HELO,
		.re = {
			.str = "("DOMAIN")"
		}
	},
	[SMTP_CMD_EHLO] = {
		.cmd = "EHLO",
		.cmd_len = sizeof("EHLO") - 1,
		.evt = SMTP_EV_EHLO,
		.re = {
			.str = "("DOMAIN")"
		}
	},
	[SMTP_CMD_MAIL] = {
		.cmd = "MAIL",
		.cmd_len = sizeof("MAIL") - 1,
		.evt = SMTP_EV_MAIL,
	},
	[SMTP_CMD_DATA] = {
		.cmd = "DATA",
		.cmd_len = sizeof("DATA") - 1,
		.evt = SMTP_EV_DATA,
	},
	[SMTP_CMD_RCPT] = {
		.cmd = "RCPT",
		.cmd_len = sizeof("RCPT") - 1,
		.evt = SMTP_EV_RCPT,
	},
	[SMTP_CMD_RSET] = {
		.cmd = "RSET",
		.cmd_len = sizeof("RSET") - 1,
		.evt = SMTP_EV_RSET,
	},
	[SMTP_CMD_VRFY] = {
		.cmd = "VRFY",
		.cmd_len = sizeof("VRFY") - 1,
		.evt = SMTP_EV_VRFY,
	},
	[SMTP_CMD_QUIT] = {
		.cmd = "QUIT",
		.cmd_len = sizeof("QUIT") - 1,
		.evt = SMTP_EV_QUIT,
	},
};

// TODO: destructor
__attribute__((constructor))
static void smtp_data_internal_init(void)
{
	for (uint32_t i = 0; i < SMTP_CMD_LAST; i++) {
		struct smtp_reg *r = &smtp_cmd_arr[i].re;
		const char *err;
		int err_off;

		// TODO: determine all and remore this check
		if (r->str == NULL)
			continue;

		r->re = pcre_compile(r->str, PCRE_CASELESS, &err, &err_off, NULL);
		if (r->re == NULL) {
			slog_e("smtp_data: incorrect regular expression for cmd %d", i);
			abort();
		}
	}
};

static enum smtp_cmd determine_cmd(struct buf *buf)
{
	const char *cmd = buf_get_data(buf);
	uint32_t len = buf_get_len(buf);
	enum smtp_cmd smtp_cmd = SMTP_CMD_EMPTY;

	if (len < SMTP_CMD_MIN_LEN)
		return smtp_cmd;

	for (uint32_t i = 0; i < SMTP_CMD_LAST; i++) {
		if (smtp_cmd_arr[i].cmd == NULL)
			continue;

		if (memcmp(smtp_cmd_arr[i].cmd, cmd, smtp_cmd_arr[i].cmd_len) == 0) {
			if (i == SMTP_CMD_EHLO) {
				smtp_cmd = SMTP_CMD_HELO;
			} else
				smtp_cmd = i;
			break;
		}
	}

	return smtp_cmd;
}

void smtp_data_init(struct smtp_data *s_data, const char *name)
{
	*s_data = (struct smtp_data) {
		.state = SMTP_ST_INIT,
		.name = name,
	};

	char info[SMTP_RET_MSG_LEN];
	char *msg = "Simple Mail Transfer Service Ready";
	int len = snprintf(info, sizeof(info), "%s %s", name, msg);
	if (len < 0)
		abort();

	SMTP_DATA_FORM_ANSWER(s_data, 220, info);
}

void smtp_data_reset(struct smtp_data *s_data)
{
	s_data->cur_cmd = SMTP_CMD_EMPTY;

	s_data->client.data = NULL;
	s_data->client.len = 0;
	if (s_data->client.domain != NULL) {
		free(s_data->client.domain);
		s_data->client.domain = NULL;
	}

	s_data->answer.ret = 0;
	s_data->answer.ret_msg_len = 0;
}

void smtp_data_destroy(struct smtp_data *s_data)
{
	if (s_data->client.domain != NULL)
		free(s_data->client.domain);
}

int smtp_data_process(struct smtp_data *s_data, struct buf *msg)
{
	te_smtp_event evt;
	struct smtp_cmd_info *info;

	s_data->cur_cmd = determine_cmd(msg);

	slog_d("smtp_data: process msg from client: cmd %d", s_data->cur_cmd);

	assert(s_data->cur_cmd < SMTP_CMD_LAST);

	info = &smtp_cmd_arr[s_data->cur_cmd];
	s_data->client.data = buf_get_data(msg) + info->cmd_len;
	s_data->client.len = buf_get_len(msg) - info->cmd_len;

	if (s_data->cur_cmd == SMTP_CMD_EMPTY) {
		if (buf_get_len(msg) > sizeof("\r\n")) {
			evt = SMTP_EV_DATA_RCV;
		} else {
			s_data->client.data = NULL;
			s_data->client.len = 0;

			s_data->answer.ret_msg_len = 0;
			return 0;
		}
	} else
		evt = smtp_cmd_arr[s_data->cur_cmd].evt;

	s_data->state = smtp_step(s_data->state, evt, s_data);

	if (s_data->state == SMTP_ST_ST_ERR)
		return -1;

	if (s_data->state == SMTP_ST_DONE)
		return 1;

	return 0;
}

