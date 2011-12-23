#include <czmq.h>
#include <stdlib.h>
#include <uuid/uuid.h>
#include <string.h>
#include <unistd.h>

#include "astronaut.h"

#define MAX_VHOST_LEN 24
#define SECRET_LEN 32
#define SID_LEN 32
#define MAX_IDENTITY_LEN 4 + MAX_VHOST_LEN + SECRET_LEN + SID_LEN
#define SA_TOKEN_LEN 128
#define UUID_LEN 36
#define CMD_LEN 4
#define REQUEST_TIMEOUT  1000
#define RESPONSE_TIMEOUT 5000
#define LISTENER_TIMEOUT 1000

astronaut_t* astronaut_new(const char* addr, const char *vhost, const char *secret)
{
	astronaut_t* self;
	uuid_t uuid;
	int addr_len = strlen(addr);
	char* sid = (char*)malloc(UUID_LEN * sizeof(char));
	char identity[MAX_IDENTITY_LEN];
  
	self = (astronaut_t*)malloc(sizeof(astronaut_t));
	if (!self)
		return NULL;

	pthread_mutex_init(&self->lmtx, NULL);
	pthread_mutex_init(&self->tmtx, NULL);

	self->running = 0; // not running, ready to run
  
	self->addr = (char*)malloc(addr_len * sizeof(char));
	memcpy(self->addr, addr, addr_len * sizeof(char));
	
	uuid_generate(uuid);
	uuid_unparse_lower(uuid, sid);
  
	self->ctx = zctx_new();
	self->req = zsocket_new(self->ctx, ZMQ_REQ);
	sprintf(identity, "req:%s:%s:%s", vhost, secret, sid);
	zsockopt_set_identity(self->req, identity);

	self->dlr = zsocket_new(self->ctx, ZMQ_DEALER);
	sprintf(identity, "dlr:%s:%s:%s", vhost, secret, sid);
	zsockopt_set_identity(self->req, identity);
  
	free(sid);

	// TODO: register signal handlers
  
	return self;
}

int astronaut_connect(astronaut_t* self)
{
	return zsocket_connect(self->req, self->addr);
}

zmsg_t* astronaut_request(astronaut_t* self, char* request)
{
	if (!request || !self)
		return NULL;

	zmq_pollitem_t items[] = {{self->req, 0, ZMQ_POLLOUT|ZMQ_POLLIN, 0}};
	zmsg_t *res = NULL, *req = NULL;

	pthread_mutex_lock(&self->tmtx);

	int rc = zmq_poll(items, 1, REQUEST_TIMEOUT * ZMQ_POLL_MSEC);
	if (rc == 0 && items[0].revents & ZMQ_POLLOUT) {
		zmsg_t *req = zmsg_new();
		zmsg_addstr(req, request);
		zmsg_send(&req, self->req);
		zmsg_destroy(&req);
	
		rc = zmq_poll(items, 1, RESPONSE_TIMEOUT * ZMQ_POLL_MSEC);
		if (rc == 0 && items[0].revents & ZMQ_POLLIN) {
			res = zmsg_recv(self->req);
		}
	}
  
	pthread_mutex_lock(&self->tmtx);
	return res;
}

char* astronaut_get_single_access_token(astronaut_t* self, const char* perm)
{
	if (!perm || !self)
		return NULL;
  
	zmsg_t* res = NULL;
	char* token = NULL;
	char* cmd = (char*)malloc((strlen(perm) + CMD_LEN) * sizeof(char));

	sprintf(cmd, "SAT %s", perm);
	res = astronaut_request(self, cmd);

	if (!res)
		return NULL;
	  
	token = zmsg_popstr(res);
	free(cmd);

	return token;
}

int astronaut_trigger(astronaut_t* self, char* data)
{
	if (!data || !self)
		return -1;

	int rc;
	char* rcs = NULL;
	zmsg_t* res = NULL;
	char* payload = (char*)malloc((strlen(data) + CMD_LEN) * sizeof(char));

	sprintf(payload, "TRG %s", data);
	res = astronaut_request(self, data);

	if (!res)
		return -1;

	rcs = zmsg_popstr(res);
	sscanf(rcs, "%d", &rc);
	return rc;
}

int astronaut_listen(astronaut_t* self, astronaut_listener_t listener)
{
	if (!self)
		return -1;
	if (self->running == 1)
		return E_RUNNING; // is running
  
	int rc = zsocket_connect(self->dlr, self->addr);
	if (rc != 0)
		return rc;

	char* payload = NULL;
	zmsg_t* res = NULL;
	zmq_pollitem_t items[] = {{self->req, 0, ZMQ_POLLIN, 0}};

	self->running = 1; // running
  
	while (1) {
		if (self->running < 1) { // stop
			break;
		}
	
		int rc = zmq_poll(items, 1, LISTENER_TIMEOUT * ZMQ_POLL_MSEC);
		if (rc == -1)
			continue;

		if (items[0].revents & ZMQ_POLLIN) {
			res = zmsg_recv(self->req);
			payload = zmsg_popstr(res);
			listener(self, payload);
		}
	}

	self->running = -1; // stopped
	zmsg_destroy(&res);
	return 0;
}

void astronaut_stop_listening(astronaut_t* self)
{
	if (self->running < 1)
		return;
  
	pthread_mutex_lock(&self->lmtx);
	self->running = 0;
	while (self->running != -1)
		usleep(1000);
	pthread_mutex_unlock(&self->lmtx);
}

void astronaut_destroy(astronaut_t** self_p)
{
	if (!*self_p)
		return;
  
	astronaut_t* self = *self_p;
  
	astronaut_stop_listening(self);
	zctx_destroy(&self->ctx);
	pthread_mutex_destroy(&self->lmtx);
	pthread_mutex_destroy(&self->tmtx);
	free(self);
	*self_p = NULL;
}
