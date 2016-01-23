#include "msg.h"
#include "server_types.h"

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pcre.h>
#include <unistd.h>

#define HDR_FROM	"From"
#define HDR_TO		"To"
#define HDR_MSG_ID	"Message-ID"
#define HDR_DATE	"Date"
#define HDR_SUBJ	"Subject"

static char *email_hdr_name[EMAIL_MSG_HDR_LAST] = {
	[EMAIL_MSG_HDR_FROM] =		HDR_FROM,
	[EMAIL_MSG_HDR_TO] =		HDR_TO,
	[EMAIL_MSG_HDR_MSG_ID] =	HDR_MSG_ID,
	[EMAIL_MSG_HDR_DATE] =		HDR_DATE,
	[EMAIL_MSG_HDR_SUBJECT] =	HDR_SUBJ
};

static pcre *hdr_line;

__attribute__((constructor))
static void email_internal_init(void)
{
	const char *err;
	int err_off;
	char *str_hdr_line = "^([\\w\\d-]+):.*?\r\n";

	hdr_line = pcre_compile(str_hdr_line, PCRE_NEWLINE_CRLF, &err, &err_off, NULL);
	if (hdr_line == NULL) {
		slog_e("%s", "incorrect regular expression for `hdr_line'");
		abort();
	}
}

static int  pcre_match_hdr_line(const char *data, int len,
				const char **match_off, int *match_len)
{
	int ovec[24];
	int ovecsize = sizeof(ovec);
	int off;
	int rc = pcre_exec(hdr_line, 0, data, len, 0, 0, ovec, ovecsize);

	*match_len = 0;
	*match_off = NULL;

	if (rc < 0)
		return 0;

	off = ovec[2];
	*match_len = ovec[3] - ovec[2];
	*match_off = data + off;

	return ovec[1];
}

static void email_hdr_init(struct email_hdr *h, enum email_hdr_type type)
{
	int prealloc = 128;

	buf_init(&h->hdr, prealloc);
	buf_append(&h->hdr, email_hdr_name[type], strlen(email_hdr_name[type]));
	buf_append(&h->hdr, ": ", sizeof(": ") - 1);
	h->el_cnt = 0;
}

static void email_hdr_set_empty(struct email_hdr *h)
{
	buf_reset(&h->hdr);
	h->el_cnt = 0;
}

static bool email_hdr_is_empty(struct email_hdr *h)
{
	return buf_get_len(&h->hdr) == 0 || h->el_cnt == 0;
}

static void email_hdr_reset(struct email_hdr *h, enum email_hdr_type type)
{
	buf_reset(&h->hdr);
	buf_append(&h->hdr, email_hdr_name[type], strlen(email_hdr_name[type]));
	buf_append(&h->hdr, ": ", sizeof(": ") - 1);
	h->el_cnt = 0;
}

static void email_hdr_free(struct email_hdr *h)
{
	buf_free(&h->hdr);
}

static void email_hdr_append(struct email_hdr *h, const char *delim,
			     const char *data, int len)
{
	buf_append(&h->hdr, delim, strlen(delim));
	buf_append(&h->hdr, data, len);

	h->el_cnt++;
}

void email_init(struct email *e, uint32_t prealloc)
{
	for (uint32_t i = 0; i < EMAIL_MSG_HDR_LAST; i++) {
		email_hdr_init(&e->hdrs[i], i);
	}

	buf_init(&e->body, prealloc);
}

void email_reset(struct email *e)
{
	for (uint32_t i = 0; i < EMAIL_MSG_HDR_LAST; i++) {
		email_hdr_reset(&e->hdrs[i], i);
	}

	buf_reset(&e->body);
}

void email_destroy(struct email *e)
{
	for (uint32_t i = 0; i < EMAIL_MSG_HDR_LAST; i++) {
		email_hdr_free(&e->hdrs[i]);
	}

	buf_free(&e->body);
}

void email_add_from(struct email *e, const char *from, int len)
{
	email_hdr_append(&e->hdrs[EMAIL_MSG_HDR_FROM], "", from, len);
}

int email_add_rcpt(struct email *e, const char *rcpt, int len)
{
	struct email_hdr *h = &e->hdrs[EMAIL_MSG_HDR_TO];

	if (h->el_cnt == 0) {
		email_hdr_append(h, "", rcpt, len);
	} else if (h->el_cnt < EMAIL_RCPT_CNT_MAX) {
		email_hdr_append(h, EMAIL_RCP_DELIM, rcpt, len);
	} else
		return -1;

	return 0;
}

void email_append_body(struct email *e, const char *data, int len)
{
	if (len == 0 || data == NULL)
		return;

	buf_append(&e->body, data, len);
}

static int email_hdr_date_create(struct email_hdr *h)
{
	char timestamp[255];
	time_t t = time(NULL);
	struct tm *tmp = localtime(&t);
	if (tmp == NULL) {
		slog_e("Can't get localtime: %s", strerror(errno));
		return -1;
	}

	if (strftime(timestamp, sizeof(timestamp),
		     "%a, %e %b %Y %T %z", tmp) == 0) {
		slog_e("Strftime failed: %s", strerror(errno));
		return -1;
	}
	email_hdr_append(h, "", timestamp, sizeof(timestamp) - 1);

	return 0;
}

static int email_hdr_msg_id_create(struct email_hdr *h)
{
	char msg_id[EMAIL_HDR_MAX_LEN];

	int cnt = snprintf(msg_id, sizeof(msg_id), "<%ld-%d-%ld",
			   time(NULL) | rand(), rand(), time(NULL));

	if (cnt <= 0)
		return -1;

	email_hdr_append(h, "", msg_id, cnt);

	return 0;
}

static int email_write_to_file(struct email *e, const char *path)
{
	char fname[PATH_MAX];

	if (snprintf(fname, sizeof(fname), "%s/%ld-%d-%d.eml",
		 path, time(NULL), getpid(), rand()) <= 0)
		abort();

	slog_i("store email to %s", fname);

	FILE *f = fopen(fname, "w");
	if (!f) {
		slog_e("Can't open message %s: %s", fname, strerror(errno));
		return -1;
	}

	for (uint32_t i = 0; i < EMAIL_MSG_HDR_LAST; i++) {
		if (email_hdr_is_empty(&e->hdrs[i]) == true)
			continue;

		switch (i) {
		case EMAIL_MSG_HDR_FROM:
		case EMAIL_MSG_HDR_TO:
		case EMAIL_MSG_HDR_SUBJECT:
			break;
		case EMAIL_MSG_HDR_DATE:
			if (email_hdr_date_create(&e->hdrs[i]) != 0)
				abort();
			break;
		case EMAIL_MSG_HDR_MSG_ID:
			if (email_hdr_msg_id_create(&e->hdrs[i]) != 0)
				abort();
			break;
		default:
			abort();
		}

		fprintf(f, "%.*s\r\n",
		       buf_get_len(&e->hdrs[i].hdr),
		       buf_get_data(&e->hdrs[i].hdr));
	}

	fprintf(f, "%.*s\r\n",
		   buf_get_len(&e->body), buf_get_data(&e->body));
	fclose(f);

	return 0;
}

static void email_prepare_hdrs(struct email *e)
{
	const char *email = buf_get_data(&e->body);
	int len = buf_get_len(&e->body);

	while (len > 0) {
		const char *match_hdr;
		int match_hdr_len;

		int off_end = pcre_match_hdr_line(email, len, &match_hdr, &match_hdr_len);

		if (off_end == 0)
			break;

		email += off_end;
		len -= off_end;

		if (match_hdr_len == 0)
			continue;

		slog_d("parse email hdrs: found hdr %.*s", match_hdr_len, match_hdr);

		for (uint32_t i = 0; i < EMAIL_MSG_HDR_LAST; i++) {
			if (strncasecmp(match_hdr, email_hdr_name[i], match_hdr_len) == 0) {
				email_hdr_set_empty(&e->hdrs[i]);
				if (i == EMAIL_MSG_HDR_TO) {
					buf_append(&(e->hdrs[i]).hdr, email - off_end, off_end);
				}
				break;
			}
		}
	};
}

static bool email_hdr_rcpt_is_internal(struct email_hdr *hdr, const char *domain)
{
	char *c;
	char *rcpt = buf_get_data(&hdr->hdr);
	int len = buf_get_len(&hdr->hdr);
	bool internal = true;

	c = rcpt;
	slog_d("rcpt %.*s domain %s", len, rcpt, domain);
	while ((c = strchr(c, '@')) != NULL && c < (rcpt + len)) {
		char *start = ++c;

		while (*c != '@' && (isalpha(*c) || *c == '.')) {
			if (c == (rcpt + len - 1))
				break;
			c++;
		}

		int cnt = c - start;
		if (cnt == 0)
			break;

		slog_d("TO: found domain: %.*s", cnt, c);

		if (strncasecmp(start, domain, cnt) != 0) {
			internal = false;
			break;
		}
	}
	return internal;
}

int email_store(struct email *e, const char *domain,
		const char *int_path, const char *ext_path)
{
	const char *path;
	email_prepare_hdrs(e);

	bool internal = email_hdr_rcpt_is_internal(&e->hdrs[EMAIL_MSG_HDR_TO], domain);
	path = internal == true ? int_path : ext_path;

	return email_write_to_file(e, path);
}
