#include "smtp_proto.h"

#include <pcre.h>

#define NUMBER		"(?:\\d+)"
#define SNUM		"(?:\\d{1,3})"
#define DOTNUM		"(?:"SNUM"\\."SNUM"\\."SNUM"\\."SNUM")"
#define NAME		"(?:[a-zA-Z][a-zA-Z\\d\\-]*[a-zA-Z\\d])"
#define ELEMENT		"(?:"NAME"|(?:#"NUMBER")|(?:\\["DOTNUM"\\]))"
#define DOMAIN		"(?:"ELEMENT"(?:\\."ELEMENT")*)"
#define SP		"\\s"
#define CRLF		"\13\10"

// TODO: SP + CRLF
struct smtp_cmd_info {
	char *cmd;
	uint8_t cmd_len;

	struct smtp_reg {
		pcre *re;
		const char *str;
	} re;
};

struct smtp_cmd_info smtp_cmd_arr[SMTP_CMD_LAST] = {
	[SMTP_CMD_HELO] = {
		.cmd = "HELO",
		.cmd_len = sizeof("HELO"),
		.re = {
			.str = "("DOMAIN")"
		}
	},
	[SMTP_CMD_EHLO] = {
		.cmd = "EHLO",
		.cmd_len = sizeof("EHLO"),
	},
	[SMTP_CMD_MAIL] = {
		.cmd = "MAIL",
		.cmd_len = sizeof("MAIL"),
	},
	[SMTP_CMD_DATA] = {
		.cmd = "DATA",
		.cmd_len = sizeof("DATA"),
	},
	[SMTP_CMD_RCPT] = {
		.cmd = "RCPT",
		.cmd_len = sizeof("RCPT"),
	},
	[SMTP_CMD_RSET] = {
		.cmd = "RSET",
		.cmd_len = sizeof("RSET"),
	},
	[SMTP_CMD_VRFY] = {
		.cmd = "VRFY",
		.cmd_len = sizeof("VRFY"),
	},
	[SMTP_CMD_QUIT] = {
		.cmd = "QUIT",
		.cmd_len = sizeof("QUIT"),
	},
};

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
// TODO: destructor

static const char *cmd_parse(pcre *re, const char *data, int data_len, int *len) {
	int ovec[24];
	int ovecsize = sizeof(ovec);

	int rc = pcre_exec(re, 0, data, (int)data_len, 0, 0, ovec, ovecsize);

	int off = ovec[0];
	*len = ovec[1] - ovec[0];
	if (rc < 0) {
		slog_e("Invalid command came, data == '%.*s'", (int)data_len, data);
		*len = -1;
		return NULL;
	}

	if (*len == 0) {
		slog_d("%s", "Empty addr found cmd");
		return "";
	}

	slog_d("smtp_data: cmd_parsed: '%.*s'", *len, data + off);
	return data + off;
}

static enum smtp_cmd determine_cmd(struct buf *buf)
{
	const char *cmd = buf_get_data(buf);
	uint32_t len = buf_get_len(buf);
	enum smtp_cmd smtp_cmd = SMTP_CMD_EMPTY;

	if (len < SMTP_CMD_MIN_LEN)
		return smtp_cmd;

	for (uint32_t i = 0; i < SMTP_CMD_LAST; i++) {
		if (strncmp(smtp_cmd_arr[i].cmd, cmd, smtp_cmd_arr[i].cmd_len) == 0) {
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
}

void smtp_data_destroy(struct smtp_data *s_data)
{
	(void)s_data;
}


int smtp_data_process(struct smtp_data *s_data, struct buf *msg)
{
	enum smtp_cmd smtp_cmd = determine_cmd(msg);
	slog_d("smtp_data: process msg from client: cmd %d", smtp_cmd);

	switch (smtp_cmd) {
	case SMTP_CMD_EMPTY:
	case SMTP_CMD_HELO:
	case SMTP_CMD_EHLO:
	case SMTP_CMD_MAIL:
	case SMTP_CMD_DATA:
	case SMTP_CMD_RCPT:
	case SMTP_CMD_RSET:
	case SMTP_CMD_VRFY:
	case SMTP_CMD_QUIT: {
		struct smtp_cmd_info *info = &smtp_cmd_arr[smtp_cmd];
		if (info->re.re == NULL)
			break;

		const char *args_str = buf_get_data(msg) + info->cmd_len;
		int len = buf_get_len(msg) - info->cmd_len;
		const char *args = cmd_parse(info->re.re, args_str, len, &len);
		if (args == NULL) {
			// TODO: go to err state
			slog_d("incorret args for cmd %d", smtp_cmd);
			break;
		}

		slog_d("accepted cmd and args %s", args);
		break;
	}
	default:
		abort();
	};

	struct smtp_msg *ans = &s_data->answer;
	ans->ret = 200;
	ans->ret_msg_len = snprintf(ans->ret_msg, sizeof(ans->ret_msg), "%s", "OK");

	if (ans->ret_msg_len == -1) {
		slog_e("%s", "smtp_data: err in snprintf");
		return -1;
	}

	return 0;
}

