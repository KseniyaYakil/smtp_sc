#include "log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

pthread_t thread_logger;
FILE* logger_pipe = 0;
pthread_mutex_t logger_lock;
int* pipes;

log_level cur_lvl;

static void *_thread_logger_func(void* arg_log_base_filename);

int start_logger(const char* log_filename_base) {
	if (logger_pipe) {
		slog_w("%s", "logger already has been started");
		return 0;
	}

	pthread_mutex_init(&logger_lock, NULL);

	pipes = malloc(sizeof(int) * 2);
	if (pipe(pipes)) {
		perror("pipe() failed");
		return -1;
	}

	if (pthread_create(&thread_logger, NULL, _thread_logger_func, (void*)log_filename_base) != 0) {
		perror("pthread_create() failed");
		return -1;
	}

	if (!(logger_pipe = fdopen(pipes[1], "w"))) {
		perror("Can't open log pipe");

		return -1;
	}

	return 0;
}

int stop_logger(void) {
	int result = 0;

	if (logger_pipe) {
		pthread_mutex_lock(&logger_lock);

		if (fprintf(logger_pipe, "%d %d %lu %s", LS_STOP, DEBUG, (unsigned long)4, "stop") > 0) {
			fflush(logger_pipe);
		} else {
			perror("fprintf");

			result = -1;
		}

		pthread_mutex_unlock(&logger_lock);
	} else {
		slog_w("%s", "Logger hasn't been started");
	}

	if (thread_logger) {
		pthread_join(thread_logger, NULL);
	}

	for (int i = 0; i < 2; i++) {
		close(pipes[i]);
	}

	return result;
}

int send_log_message(log_level log_lvl, char* message) {
	int result = 0;

	if (logger_pipe) {
		pthread_mutex_lock(&logger_lock);

		if (fprintf(logger_pipe, "%d %d %lu %s", LS_CONTINUE, log_lvl, strlen(message), message) > 0) {
			fflush(logger_pipe);
		} else {
			perror("fprintf");
			result = -1;
		}

		pthread_mutex_unlock(&logger_lock);
	} else
		fprintf(stderr, "%s", message);

	return result;
}

static void *_thread_logger_func(void* arg_log_base_filename) {
	char* log_filename_base = (char*)arg_log_base_filename;

	char* log_filename = malloc((strlen(log_filename_base) + 5) * sizeof(char));
	sprintf(log_filename, "%s", log_filename_base);
	FILE* out = fopen(log_filename, "a");
	if (!out) {
		fprintf(stderr, "Couldn't open log file '%s', error: %s\n", log_filename, strerror(errno));
		free(log_filename);

		pthread_exit(NULL);
	}
	sprintf(log_filename, "%s.err", log_filename_base);
	FILE* err = fopen(log_filename, "a");
	if (!err) {
		fprintf(stderr, "Couldn't open log file '%s', error: %s\n", log_filename, strerror(errno));
		free(log_filename);

		pthread_exit(NULL);
	}

	free(log_filename);

	FILE* stream = fdopen(pipes[0], "r");
	if (!stream) {
		perror("Can't open parent pipe");

		pthread_exit(NULL);
	}

	int logger_signal;
	int log_level;
	unsigned long length;

	while ( fscanf(stream, "%d %d %lu ", &logger_signal, &log_level, &length) == 3 ) {
		if (logger_signal == LS_STOP) {
			break;
		}

		char* message = malloc((length + 1) * sizeof (char));
		fread(message, sizeof(char), length, stream);
		message[length] = '\0';
		fprintf(out, "%s", message);
		fflush(out);
		if (log_level >= WARN) {
			fprintf(err, "%s", message);
			fflush(err);
		}
		free(message);
	}

	fclose(stream);
	fclose(err);
	fclose(out);

	pthread_exit(NULL);
}

