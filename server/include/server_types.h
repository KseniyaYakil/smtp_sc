
#ifndef _SERVER_TYPES_H_
#define _SERVER_TYPES_H_

#include <errno.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

enum server_log_lvl {
	SERVER_LOG_LVL_ERR = 0,
	SERVER_LOG_LVL_INF = 1,
	SERVER_LOG_LVL_DEB = 2,

	SERVER_LOG_LVL_LAST,
};

struct server_conf {
	long int port;
	long int thread_cnt;

	enum server_log_lvl log_lvl;
	const char *mail_dir;
	const char *queue_dir;
};

struct buf {
	char *data;
	uint32_t len; // used
	uint32_t size; //alloced size
};

extern struct server_conf conf;

//TODO: add to common
void buf_init(struct buf *buf, uint32_t prealloc);
void buf_reset(struct buf *buf);
void buf_append(struct buf *buf, const char *data, uint32_t len);
char *buf_get_data(struct buf *buf);
uint32_t buf_get_len(struct buf *buf);
int buf_copy(struct buf *buf, char **data_p, uint32_t *len);
void buf_free(struct buf *buf);

#define MAX_CLIENTS	1024

#define slog_log(lvl, format_, ...) {		\
	if (lvl <= conf.log_lvl) {		\
		const char *prefix;		\
		switch (lvl) {			\
		case SERVER_LOG_LVL_ERR:	\
			prefix = "ERR";		\
			break;			\
		case SERVER_LOG_LVL_INF:	\
			prefix = "INF";		\
			break;			\
		case SERVER_LOG_LVL_DEB:	\
			prefix = "DEB";		\
			break;			\
		default:			\
			prefix = NULL;		\
			break;			\
		}				\
						\
		if (prefix != NULL)		\
			printf("%s: "format_"\n", prefix, __VA_ARGS__);\
	}					\
}

#define slog_d(...)	slog_log(SERVER_LOG_LVL_DEB, __VA_ARGS__);
#define slog_i(...)	slog_log(SERVER_LOG_LVL_INF, __VA_ARGS__);
#define slog_e(...)	slog_log(SERVER_LOG_LVL_ERR, __VA_ARGS__);

#endif // _SERVER_TYPES_H_

