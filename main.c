#include <unistd.h>
#include <stdlib.h>
#include "common.h"
#include "ipc.h"
#include "pa1.h"

#define CMDP_CHILD_COUNT "-p"

struct channel {
	int input;
	int output;
};

struct common_data {
	int count;
	pid_t parent;
	int log, pipes_log;
} common;

struct process_data {
	uint32_t id;
	struct channel* channels;
} *data;

int init_data(int count)
{
	data = malloc(sizeof(struct process_data) * count);
	if(data == NULL) return -1;

	for(int i = 0; i < count; ++i) {
		data[i].id = i;
		data[i].channels = malloc(sizeof(struct channel) * count);
	}
	return 0;
}

int init_channels(int count)
{
	int tmpfd[2];
	for(int i = 0; i < count; ++i) {
		for(int j = 0; j < count; ++j) {
			if(i == j) {
				/* no loopback :) */
				data[i].channels[j].output = -1;
				data[i].channels[j].input = -1;
			}
			if(pipe(&tmpfd) == 0) {
				data[i].channels[j].output = tmpfd[0];
				data[j].channels[i].input = tmpfd[1];
			} else {
				return -1;
			}
		}
	}
	return 0;
}

void close_unneeded_channels(int who, int count)
{
	for(int i = 0; i < count; ++i) {
		for(int j = 0; j < count; ++j) {
			if(j == i) continue;
			if(i == who) continue;
			close(data[i].channels[j].input);
			close(data[i].channels[j].output);
		}
	}
}

int child_work(int i, int count, pid_t parent)
{
	close_unneeded_channels(i, count);



	return EXIT_SUCCESS;
}


int main(int argc, char* argv[])
{
	int children;
	pid_t parent_pid = getpid();

	if(argc != 3) return EXIT_FAILURE;
	if(strncmp(argv[1], CMDP_CHILD_COUNT, sizeof(CMDP_CHILD_COUNT)) == 0) {
		children = atoi(argv[2]);
		if(children < 1) return EXIT_FAILURE;
	} else {
		return EXIT_FAILURE;
	}

	common.log = open("events.log", O_CREAT | O_WRONLY);
	common.pipes_log = open("pipes.log", O_CREAT | O_WRONLY);

	if(init_data(children + 1) != 0 || init_channels(children + 1) != 0) {
		return EXIT_FAILURE;
	}

	for(int i = 0; i < children; ++i) {
		pid_t pid = fork();

		if(pid < 0) {
			return EXIT_FAILURE;
		}

		if(pid == 0) {
			return child_work();
		} else {
			int status;
			for(int i = 0; i < children; ++i) {
				wait(&status);
			}
		}
	}

	return EXIT_SUCCESS;
}