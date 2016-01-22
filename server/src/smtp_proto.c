#include "msg.h"
#include "smtp_proto.h"

#include <assert.h>
#include <pcre.h>

#define NUMBER		"\\d+"
#define SNUM		"\\d{1,3}"
#define DOTNUM		""SNUM"\\."SNUM"\\."SNUM"\\."SNUM""
#define NAME		"[a-zA-Z][a-zA-Z\\d\\-]*[a-zA-Z\\d]"
#define ELEMENT		"(?:"NAME")|(?:#"NUMBER")|(?:\\["DOTNUM"\\])"
#define DOMAIN		"(?:"ELEMENT")(?:\\."ELEMENT")*"
#define SP		" "
#define DATA_EOF_RE	"(.*)("DATA_EOF")"

#define ASCII_CHAR	"[\\x00-\\x19\\x21-\\x7F]"
#define CHAR		ASCII_CHAR
#define STR		"(?:"CHAR"+)"
#define DOT_STRING	"(?:"STR")|(?:\\."STR")*"
#define LOCAL_PART	"(?:"DOT_STRING")"
#define MAILBOX		"(?:"LOCAL_PART")@(?:"DOMAIN")"
#define AT_DOMAIN	"@"DOMAIN""
#define ADL		"(?:"AT_DOMAIN")(?:,"AT_DOMAIN")*"
#define PATH		"\\<(?:"ADL"\\:)?(?:"MAILBOX")\\>"
#define REVERSE_PATH	PATH
#define FORWARD_PATH	PATH

struct smtp_cmd_info smtp_cmd_arr[SMTP_CMD_LAST] = {
	[SMTP_CMD_HELO] = {
		.cmd = "HELO",
		.cmd_len = sizeof("HELO") - 1,
		.evt = SMTP_EV_HELO,
		.re = {
			.str = "^"SP"("DOMAIN")"CRLF
		}
	},
	[SMTP_CMD_EHLO] = {
		.cmd = "EHLO",
		.cmd_len = sizeof("EHLO") - 1,
		.evt = SMTP_EV_EHLO,
		.re = {
			.str = "^"SP"("DOMAIN")"CRLF
		}
	},
	[SMTP_CMD_MAIL] = {
		.cmd = "MAIL",
		.cmd_len = sizeof("MAIL") - 1,
		.evt = SMTP_EV_MAIL,
		.re = {
			.str = "^"SP"FROM\\:("REVERSE_PATH")"CRLF
		}
	},
	[SMTP_CMD_DATA] = {
		.cmd = "DATA",
		.cmd_len = sizeof("DATA") - 1,
		.evt = SMTP_EV_DATA,
		.re = {
			.str = "^("CRLF")"
		}
	},
	[SMTP_CMD_RCPT] = {
		.cmd = "RCPT",
		.cmd_len = sizeof("RCPT") - 1,
		.evt = SMTP_EV_RCPT,
		.re = {
			.str = "^"SP"TO\\:("FORWARD_PATH")"CRLF
		}
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

pcre *data_eof_re;

__attribute__((constructor))
static void smtp_data_internal_init(void)
{
	const char *err;
	int err_off;

	for (uint32_t i = 0; i < SMTP_CMD_LAST; i++) {
		struct smtp_reg *r = &smtp_cmd_arr[i].re;

		if (r->str == NULL)
			continue;

		r->re = pcre_compile(r->str, PCRE_CASELESS, &err, &err_off, NULL);
		if (r->re == NULL) {
			slog_e("smtp_data: incorrect regular expression for cmd %d", i);
			abort();
		}
	}

	data_eof_re = pcre_compile(DATA_EOF_RE, PCRE_CASELESS, &err, &err_off, NULL);
	if (data_eof_re == NULL) {
		slog_e("%s", "smtp_data: incorrect regular expression for `data_oef'");
		abort();
	}
};

static enum smtp_cmd determine_cmd(struct buf *buf)
{
	const char *cmd = buf_get_data(buf);
	enum smtp_cmd smtp_cmd = SMTP_CMD_EMPTY;

	for (uint32_t i = 0; i < SMTP_CMD_LAST; i++) {
		if (smtp_cmd_arr[i].cmd == NULL)
			continue;

		if (memcmp(smtp_cmd_arr[i].cmd, cmd, smtp_cmd_arr[i].cmd_len) == 0) {
			smtp_cmd = i;
			break;
		}
	}

	return smtp_cmd;
}

void smtp_data_init(struct smtp_data *s_data, const char *name, const char *mail_dir)
{
	*s_data = (struct smtp_data) {
		.state = SMTP_ST_INIT,
		.name = name,
		.mail_dir = mail_dir
	};

	email_init(&s_data->client.email, 0);

	char info[SMTP_RET_MSG_LEN];
	char *msg = "Simple Mail Transfer Service Ready";
	int len = snprintf(info, sizeof(info), "%s %s", name, msg);
	if (len < 0)
		abort();

	SMTP_DATA_FORM_ANSWER(s_data, 220, info);
}

void smtp_data_reset(struct smtp_data *s_data)
{
	email_reset(&s_data->client.email);
}

void smtp_data_destroy(struct smtp_data *s_data)
{
	smtp_data_reset(s_data);

	if (s_data->client.domain != NULL)
		free(s_data->client.domain);

	email_destroy(&s_data->client.email);
}

void smtp_data_store_from(struct smtp_data *s_data, const char *from, int len)
{
	email_add_from(&s_data->client.email, from, len);
}

int smtp_data_add_rcpt(struct smtp_data *s_data, const char *rcpt, int len)
{
	return email_add_rcpt(&s_data->client.email, rcpt, len);
}

int smtp_data_append_email(struct smtp_data *s_data, const char *data, int len)
{
	email_append_body(&s_data->client.email, data, len);
	return 0;
}

int smtp_data_store_email(struct smtp_data *s_data)
{
	return email_store(&s_data->client.email, s_data->mail_dir);
}

int smtp_data_email_copy_tail(struct smtp_data *s_data, char *str, int len)
{
	return buf_copy_tail(&s_data->client.email.body, str, len);
}

int smtp_data_process(struct smtp_data *s_data, struct buf *msg)
{
	te_smtp_event evt;
	struct smtp_cmd_info *info;

	if (s_data->state != SMTP_ST_DATA_WAIT) {
		s_data->cur_cmd = determine_cmd(msg);
	} else
		s_data->cur_cmd = SMTP_CMD_EMPTY;

	slog_d("smtp_data: cmd from client %d", s_data->cur_cmd);

	assert(s_data->cur_cmd < SMTP_CMD_LAST);

	info = &smtp_cmd_arr[s_data->cur_cmd];
	s_data->client.data = buf_get_data(msg) + info->cmd_len;
	s_data->client.len = buf_get_len(msg) - info->cmd_len;

	if (s_data->cur_cmd == SMTP_CMD_EMPTY) {
		evt = SMTP_EV_DATA_RCV;
	} else
		evt = smtp_cmd_arr[s_data->cur_cmd].evt;

	s_data->state = smtp_step(s_data->state, evt, s_data);

	if (s_data->state == SMTP_ST_ST_ERR)
		return -1;

	if (s_data->state == SMTP_ST_DONE)
		return 1;

	return 0;
}

