
#ifndef _SERVER_TYPES_H_
#define _SERVER_TYPES_H_

#include <errno.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

extern struct server_conf conf;

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

