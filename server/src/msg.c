#include "msg.h"
#include "server_types.h"

#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static char *email_hdr_name[EMAIL_MSG_HDR_LAST] = {
	[EMAIL_MSG_HDR_FROM] =		"From",
	[EMAIL_MSG_HDR_TO] =		"To",
	[EMAIL_MSG_HDR_MSG_ID] =	"Message-ID",
	[EMAIL_MSG_HDR_DATE] =		"Date",
};

static void email_hdr_init(struct email_hdr *h, enum email_hdr_type type)
{
	int prealloc = 128;

	buf_init(&h->hdr, prealloc);
	buf_append(&h->hdr, email_hdr_name[type], strlen(email_hdr_name[type]));
	buf_append(&h->hdr, ": ", sizeof(": ") - 1);
	h->el_cnt = 0;
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

static int email_hdr_msg_id_create(struct email_hdr *h, struct email_hdr *from)
{
	char msg_id[EMAIL_HDR_MAX_LEN];
	const char *domain = strchr(buf_get_data(&from->hdr), '@');

	int cnt = snprintf(msg_id, sizeof(msg_id), "<%ld-%d-%ld%s",
			   time(NULL) | rand(), rand(), time(NULL), domain);

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
		fprintf(f, "%.*s\r\n",
		       buf_get_len(&e->hdrs[i].hdr),
		       buf_get_data(&e->hdrs[i].hdr));
	}

	fprintf(f, "%.*s\r\n",
		   buf_get_len(&e->body), buf_get_data(&e->body));
	fclose(f);

	return 0;
}

int email_store(struct email *e, const char *path)
{
	if (email_hdr_date_create(&e->hdrs[EMAIL_MSG_HDR_DATE]) != 0)
		return -1;

	if (email_hdr_msg_id_create(&e->hdrs[EMAIL_MSG_HDR_MSG_ID],
				    &e->hdrs[EMAIL_MSG_HDR_FROM]) != 0)
		return -1;

	// TODO: add detecting of Subject
	return email_write_to_file(e, path);
}
