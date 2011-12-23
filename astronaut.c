#include <czmq.h>
#include <stdlib.h>
#include <uuid/uuid.h>
#include <string.h>

#define MAX_VHOST_LEN 20
#define SECRET_LEN 32
#define SID_LEN 32
#define MAX_IDENTITY_LEN 4 + MAX_VHOST_LEN + SECRET_LEN + SID_LEN
#define SA_TOKEN_LEN 128
#define UUID_LEN 36
#define CMD_LEN 4

#define REQUEST_TIMEOUT  1000
#define RESPONSE_TIMEOUT 5000

typedef struct astronaut_t {
  zctx_t* ctx;
  void* req;
} astronaut_t;

astronaut_t* astronaut_new(const char *vhost, const char *secret)
{
  astronaut_t* self;
  uuid_t uuid;
  char* sid = (char*)malloc(UUID_LEN * sizeof(char));
  char identity[MAX_IDENTITY_LEN];

  self = (astronaut_t*)malloc(sizeof(astronaut_t));
  if (!self)
	return NULL;
	  
  self->ctx = zctx_new();
  self->req = zsocket_new(self->ctx, ZMQ_REQ);
  uuid_generate(uuid);
  uuid_unparse_lower(uuid, sid);
  sprintf(identity, "req:%s:%s:%s", vhost, secret, sid);
  zsockopt_set_identity(self->req, identity);
  free(sid);

  // TODO: register signal handlers
  
  return self;
}

int astronaut_connect(astronaut_t* self, const char* uri)
{
  return zsocket_connect(self->req, uri);
}

zmsg_t* astronaut_request(astronaut_t* self, char* request)
{
  if (!request || !self)
	return NULL;

  zmq_pollitem_t items[] = {{self->req, 0, ZMQ_POLLIN, 0}};

  int rc = zmq_poll(items, 1, REQUEST_TIMEOUT * ZMQ_POLL_MSEC);
  if (rc == -1)
	return NULL;

  if (items[0].revents & ZMQ_POLLIN) {
	  zmsg_t *msg = zmsg_new();
	  zmsg_addstr(msg, request);
	  zmsg_send(&msg, self->req);
	  zmsg_destroy(&msg);
  }

  rc = zmq_poll(items, 1, RESPONSE_TIMEOUT * ZMQ_POLL_MSEC);
  if (rc == -1)
	return NULL;

  if (items[0].revents & ZMQ_POLLIN) {
	zmsg_t *res = zmsg_recv(self->req);
	return res;
  }

  return NULL;
}

char* astronaut_get_single_access_token(astronaut_t* self, char* perm)
{
  if (!perm || !self)
	return NULL;
  
  zmsg_t* res;
  char* token;
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

  zmsg_t* res;
  char* rcs;
  int rc;
  char* payload = (char*)malloc((strlen(data) + CMD_LEN) * sizeof(char));

  sprintf(payload, "TRG %s", data);
  res = astronaut_request(self, data);

  if (!res)
	return -1;

  rcs = zmsg_popstr(res);
  sscanf(rcs, "%d", &rc);
  return rc;
}

void astronaut_listen()
{
  
}

void astronaut_destroy(astronaut_t** self_p)
{
  if (!*self_p)
	return;
	  
  astronaut_t* self = *self_p;
  zctx_destroy(&self->ctx);
  free(self);
  *self_p = NULL;
}

int main(int argc, char *argv[])
{
  astronaut_t* astronaut = astronaut_new("/vhost", "123456");
  astronaut_connect(astronaut, "tcp://127.0.0.1:3000");

  zmsg_t* msg = astronaut_request(astronaut, "Hello!");
  zmsg_destroy(&msg);

  astronaut_destroy(&astronaut);
  return 0;
}
