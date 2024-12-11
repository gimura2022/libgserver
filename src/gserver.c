#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#include <glog.h>
#include <gstd/allocators.h>

#include "gserver.h"

struct glog__logger* gserver__logger         = NULL;
struct gstd__memmanager* gserver__memmanager = NULL;

void gserver__init(struct glog__logger* logger, struct gstd__memmanager* memmaganer)
{
	gserver__logger     = logger;
	gserver__memmanager = memmaganer;
}

struct real_reciver_args {
	struct gserver__reciver_args args;
	gserver__reciver_f reciver;
};

static void* reciver_real(struct real_reciver_args* args);

void gserver__start_server(gserver__reciver_f reciver, void* custom_data, int port)
{
	glog__debug(gserver__logger, "starting server");

	int server_fd;
	struct sockaddr_in address;

	glog__trace(gserver__logger, "init server");

	glog__chaos(gserver__logger, "socket");
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		glog__die(gserver__logger, "socket faled");

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	glog__chaos(gserver__logger, "bind");
	if (bind(server_fd, (struct sockaddr*) &address, sizeof(address)) < 0)
		glog__die(gserver__logger, "bind faled");

	glog__chaos(gserver__logger, "listen");
	if (listen(server_fd, 3) < 0)
		glog__die(gserver__logger, "listen faled");

	glog__trace(gserver__logger, "init complete");
	glog__debug(gserver__logger, "server is listening...");

	struct gserver__context context = {0};
	context.is_runing = true;

	while (context.is_runing) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);

		struct real_reciver_args* recv_args =
			gserver__memmanager->allocator(sizeof(struct real_reciver_args));
		
		glog__chaos(gserver__logger, "accept");
		if ((recv_args->args.fd = accept(server_fd, (struct sockaddr*) &client_addr,
						&client_addr_len)) < 0) {
			glog__error(gserver__logger, "accept faled");
			continue;
		}

		glog__trace(gserver__logger, "found client connection");

		recv_args->args.context     = &context;
		recv_args->args.custom_data = custom_data;
		recv_args->reciver          = reciver;

		pthread_t thread_id;

		glog__trace(gserver__logger, "starting thread for reciver");
		pthread_create(&thread_id, NULL, (void *(*)(void*)) reciver_real, (void*) recv_args);
		pthread_detach(thread_id);
	}
}

static void* reciver_real(struct real_reciver_args* args)
{
	glog__trace(gserver__logger, "calling reciver");
	void* retval = (void*) ((size_t) args->reciver(&args->args));

	glog__trace(gserver__logger, "cleanging data");
	close(args->args.fd);
	gserver__memmanager->deallocator(args);

	pthread_exit(retval);
}
