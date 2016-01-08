#include "server_types.h"

#include <stdio.h>
#include <stdlib.h>

#define SERVER_USAGE "Usage: <server> <config_file>"

static config_t server_conf;

struct server_conf conf;

__attribute__((constructor))
static void server_init(void)
{
	config_init(&server_conf);
}

__attribute__((destructor))
static void server_deinit(void)
{
	config_destroy(&server_conf);
}

static int server_check_config(void)
{
	config_setting_t *system = config_lookup(&server_conf, "system");
	if (system == NULL) {
		slog_e("%s", "not `system' parametr");
		return -1;
	}

	if (config_setting_lookup_int(system, "port", &conf.port) != CONFIG_TRUE) {
		slog_e("%s", "No `port' parametr in config");
		return -1;
	}

	if (conf.port <= 0) {
		slog_e("incorrect `port' (%ld)", conf.port);
		return -1;
	}

	long int log_lvl;
	if (config_setting_lookup_int(system, "log_level", &log_lvl) != CONFIG_TRUE ||
	    log_lvl < 0 || log_lvl >= SERVER_LOG_LVL_LAST) {
		slog_e("%s", "incorrect `log_level' parametr in config");
		return -1;
	}
	conf.log_lvl = log_lvl;

	//TODO: user, group
	//TODO: check access and exist
	if (config_lookup_string(&server_conf, "mail_dir", &conf.mail_dir) != CONFIG_TRUE) {
		slog_e("%s", "incorrect `mail_dir'");
		return -1;
	}

	slog_d("Server port is %ld", conf.port);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		slog_e("%s", SERVER_USAGE);
		return -1;
	}

	FILE *conf_f = fopen(argv[1], "r");
	if (conf_f == NULL) {
		perror("Unable to open config file:");
		return -1;
	}

	if (config_read(&server_conf, conf_f) != CONFIG_TRUE) {
		slog_e("Error while config parsing: %s\n", config_error_text(&server_conf));
		return -1;
	}

	if (server_check_config() != 0) {
		slog_e("%s", "Unable to start server: incorrect config file");
		return -1;
	}

	slog_i("config `%s' is read", argv[1]);

	return 0;
}
