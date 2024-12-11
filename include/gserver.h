#ifndef _gserver_h
#define _gserver_h

#include <stdbool.h>

#include <glog.h>
#include <gstd/allocators.h>

struct gserver__context {
	bool is_runing;
};

struct gserver__reciver_args {
	int fd;

	void* custom_data;
	struct gserver__context* context;
};

typedef int (*gserver__reciver_f)(const struct gserver__reciver_args*);

extern struct glog__logger* gserver__logger;
extern struct gstd__memmanager* gserver__memmanager;

void gserver__init(struct glog__logger* logger, struct gstd__memmanager* memmaganer);
void gserver__start_server(gserver__reciver_f reciver, void* custom_data, int port);

#endif
