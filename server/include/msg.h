#ifndef _SMTP_EMAIL_MSG_H_
#define _SMTP_EMAIL_MSG_H_

#include "buf.h"

#include <stdint.h>
#include <inttypes.h>

#define EMAIL_RCPT_CNT_MAX	32
#define EMAIL_HDR_MAX_LEN	512
#define EMAIL_RCP_DELIM		","

enum email_hdr_type {
	EMAIL_MSG_HDR_FROM =	0,
	EMAIL_MSG_HDR_TO =	1,
	EMAIL_MSG_HDR_MSG_ID =	2,
	EMAIL_MSG_HDR_DATE =	3,

	EMAIL_MSG_HDR_LAST
};

struct email_hdr {
	struct buf hdr;
	int el_cnt;
};

struct email {
	struct email_hdr hdrs[EMAIL_MSG_HDR_LAST];
	struct buf body;
};

void email_init(struct email *e, uint32_t prealloc);
void email_destroy(struct email *e);
void email_reset(struct email *e);

void email_add_from(struct email *e, const char *from, int len);
int email_add_rcpt(struct email *e, const char *rcpt, int len);
void email_append_body(struct email *e, const char *data, int len);
int email_store(struct email *e, const char *path);

#endif // _SMTP_EMAIL_MSG_H_
