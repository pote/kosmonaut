#ifndef _ASTRONAUT_H_
#define _ASTRONAUT_H_

#include <pthread.h>

#define E_RUNNING 101

/**
 * Astronaut representation structure.
 */
typedef struct astronaut_t {
	zctx_t* ctx;
	void* req;
	void* dlr;
	char* addr;
	short int running;
	pthread_mutex_t lmtx; // listener mutex
	pthread_mutex_t tmtx; // trigger mutex
} astronaut_t;

/**
 * Listener's callback function type. 
 */
typedef void(*astronaut_listener_t)(astronaut_t*, char*);

/**
 * Astronaut constructor.
 * Creates new instance of the `astronaut_t` and initializes it's
 * internal sockets. The `astronaut_t` contains 2 kind of sockets:
 *
 * - ZMQ_REQ: handles authentication and tasks triggering
 * - ZMQ_DEALER: handles incoming tasks
 *
 * Memory allocated by `astronaut_new` have to be freed using the
 * `astronaut_destroy` function.
 *
 * @see astronaut_destroy
 * @see astronaut_connect
 *
 * @param addr   the server URI address
 * @param vhost  vhost to connect to
 * @param secret secret key assigned to given vhost
 * @return Configured astronaut instance
 */
astronaut_t* astronaut_new(const char* addr, const char* vhost, const char* secret);

/**
 * Astronaut destructor.
 * Stops event loops, disconnects the sockets and cleans up the context.
 * Frees all allocated memory as well.
 *
 * @param self_p memory address of an astronaut instance.
 */
void astronaut_destroy(astronaut_t** self_p);

/**
 * The REQ socket connector.
 * Establishes connection for the REQ socket only. To establish
 * connection for the DEALER the `astronaut_listen` function needs
 * to be used.
 *
 * @see astronaut_listen
 *
 * @param self an astronaut instance
 * @param uri  the URI to connect to
 * @return 0 if OK, otherwise appropriate error code
 */
int astronaut_connect(astronaut_t* self);

/**
 * Request for signle access token.
 * Gets a single access token from the WebRocket. Single access token
 * is used to authenticate the frontend client in case of using the
 * private channels.
 *
 * The single access token is generated for specified permissions -
 * which means each token can be used to access only those channels
 * matching the permission regexp.
 *
 * @param self an astronaut instance
 * @param perm permission regexp
 * @return a single access token string
 */
char* astronaut_get_single_access_token(astronaut_t* self, const char* perm);

/**
 * Trigger an WebRocket operation.
 * Sends trigger message to the WebRocket backend and waits for
 * confirmation.
 *
 * @param self an astronaut instance
 * @param data serialized data with trigger command (JSON-encoded string)
 * @return 0 if OK, otherwise appropriate error code
 */
int astronaut_trigger(astronaut_t* self, char* data);

/**
 * Starts the listener's event loop.
 * Establishes the DEALER connection and starts listener event loop for
 * it. Listener can't be activated more than once.
 *
 * @param self     an astronaut instance
 * @param callback callback function called when a message is received.
 * @return 0 if OK, otherwise appropriate error code
 */
int astronaut_listen(astronaut_t* self, astronaut_listener_t callback);

/**
 * Stops the listener.
 * If a listener's event loop is running, then stops it and cleans up
 * its status.
 *
 * @param self an astronaut instance
 */
void astronaut_stop_listening(astronaut_t* self);

#endif /* _ASTRONAUT_H_ */
