#include "conn.h"
#include "server_types.h"

#include <assert.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_USAGE "Usage: <server> <config_file>"
#define THREAD_CNT_DEFAULT 3

static config_t server_conf;
static char *hostname_default = "SMTP_SERVER";
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

static int server_parse_config(void)
{
	struct stat mail_dir_st;
	struct stat log_file_st;
	long int log_lvl;
	struct stat queue_dir_st;
	const char *user_group;
	struct passwd *pwd;
	struct group *gr;
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

	if (config_lookup_int(&server_conf, "thread_cnt", &conf.thread_cnt) != CONFIG_TRUE)
		conf.thread_cnt = THREAD_CNT_DEFAULT;

	if (config_lookup_string(&server_conf, "hostname", &conf.hostname) != CONFIG_TRUE) {
		slog_d("no `hostname' parametr in config. use default: %s'", hostname_default);
		conf.hostname = hostname_default;
	}

	if (config_setting_lookup_int(system, "log_level", &log_lvl) != CONFIG_TRUE ||
	    log_lvl < 0 || log_lvl >= LOG_LVL_LAST) {
		slog_e("%s", "incorrect `log_level' parametr in config");
		return -1;
	}
	conf.log_lvl = log_lvl;
	cur_lvl = log_lvl;

	if (config_lookup_string(&server_conf, "log_file", &conf.log_file) != CONFIG_TRUE) {
		slog_e("%s", "incorrect `log_file'");
		return -1;
	}

	if (stat(conf.log_file, &log_file_st) != 0) {
		slog_e("incorrect log file: %s", strerror(errno));
		return -1;
	}

	if (config_lookup_string(&server_conf, "mail_dir", &conf.mail_dir) != CONFIG_TRUE) {
		slog_e("%s", "incorrect `mail_dir'");
		return -1;
	}

	if (stat(conf.mail_dir, &mail_dir_st) != 0) {
		slog_e("incorrect mail dir: %s", strerror(errno));
		return -1;
	}

	if (config_lookup_string(&server_conf, "queue_dir", &conf.queue_dir) != CONFIG_TRUE) {
		slog_e("%s", "incorrect `queue_dir'");
		return -1;
	}

	if (stat(conf.queue_dir, &queue_dir_st) != 0) {
		slog_e("incorrect queue dir: %s", strerror(errno));
		return -1;
	}

	if (config_setting_lookup_string(system, "user", &user_group) != CONFIG_TRUE) {
		slog_e("%s", "No `user' parametr in config");
		return -1;
	}

	if ((pwd = getpwnam(user_group)) == NULL) {
		slog_e("user %s doesn't exist or error occured", user_group);
		return -1;
	}

	if (config_setting_lookup_string(system, "group", &user_group) != CONFIG_TRUE) {
		slog_e("%s", "No `group' parametr in config");
		return -1;
	}

	if ((gr = getgrnam(user_group)) == NULL) {
		slog_e("group %s doesn't exist or error occured", user_group);
		return -1;
	}

	if (setgid(gr->gr_gid) != 0 ||
	    setuid(pwd->pw_uid) != 0) {
		slog_e("unable to change to user and group from config: %s", strerror(errno));
		return -1;
	}

	if (mail_dir_st.st_uid != pwd->pw_uid || mail_dir_st.st_gid != gr->gr_gid) {
		slog_e("access denied to %s", conf.mail_dir);
		return -1;
	}

	if (queue_dir_st.st_uid != pwd->pw_uid || queue_dir_st.st_gid != gr->gr_gid) {
		slog_e("access denied to %s", conf.queue_dir);
		return -1;
	}

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

	if (server_parse_config() != 0) {
		slog_e("%s", "Unable to start server: incorrect config file");
		return -1;
	}

	slog_i("config `%s' is correct. Ready to start server", argv[1]);

	run_server();

	return 0;
}
