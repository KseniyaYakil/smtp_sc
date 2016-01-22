#ifndef _SERVER_TYPES_H_
#define _SERVER_TYPES_H_

#include "log.h"

#include <errno.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

struct server_conf {
	long int port;
	long int thread_cnt;

	log_level log_lvl;
	const char *log_file;
	const char *mail_dir;
	const char *queue_dir;
	const char *hostname;
};

extern struct server_conf conf;

#define MAX_CLIENTS	1024

#endif // _SERVER_TYPES_H_

