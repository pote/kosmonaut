#ifndef _KOSMONAUT_H_
#define _KOSMONAUT_H_

#include <pthread.h>

#define E_RUNNING 101

/**
 * Kosmonaut representation structure.
 */
typedef struct kosmonaut_t {
	zctx_t* ctx;
	void* req;
	void* dlr;
	char* addr;
	short int running;
	pthread_mutex_t lmtx; // listener mutex
	pthread_mutex_t tmtx; // trigger mutex
} kosmonaut_t;

/**
 * Listener's callback function type. 
 */
typedef void(*kosmonaut_listener_t)(kosmonaut_t*, char*);

/**
 * Kosmonaut constructor.
 * Creates new instance of the `kosmonaut_t` and initializes it's
 * internal sockets. The `kosmonaut_t` contains 2 kind of sockets:
 *
 * - ZMQ_REQ: handles authentication and tasks triggering
 * - ZMQ_DEALER: handles incoming tasks
 *
 * Memory allocated by `kosmonaut_new` have to be freed using the
 * `kosmonaut_destroy` function.
 *
 * @see kosmonaut_destroy
 * @see kosmonaut_connect
 *
 * @param addr   the server URI address
 * @param vhost  vhost to connect to
 * @param secret secret key assigned to given vhost
 * @return Configured kosmonaut instance
 */
kosmonaut_t* kosmonaut_new(const char* addr, const char* vhost, const char* secret);

/**
 * Kosmonaut destructor.
 * Stops event loops, disconnects the sockets and cleans up the context.
 * Frees all allocated memory as well.
 *
 * @param self_p memory address of an kosmonaut instance.
 */
void kosmonaut_destroy(kosmonaut_t** self_p);

/**
 * The REQ socket connector.
 * Establishes connection for the REQ socket only. To establish
 * connection for the DEALER the `kosmonaut_listen` function needs
 * to be used.
 *
 * @see kosmonaut_listen
 *
 * @param self an kosmonaut instance
 * @param uri  the URI to connect to
 * @return 0 if OK, otherwise appropriate error code
 */
int kosmonaut_connect(kosmonaut_t* self);

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
 * @param self an kosmonaut instance
 * @param perm permission regexp
 * @return a single access token string
 */
char* kosmonaut_get_single_access_token(kosmonaut_t* self, const char* perm);

/**
 * Trigger an WebRocket operation.
 * Sends trigger message to the WebRocket backend and waits for
 * confirmation.
 *
 * @param self an kosmonaut instance
 * @param data serialized data with trigger command (JSON-encoded string)
 * @return 0 if OK, otherwise appropriate error code
 */
int kosmonaut_trigger(kosmonaut_t* self, char* data);

/**
 * Starts the listener's event loop.
 * Establishes the DEALER connection and starts listener event loop for
 * it. Listener can't be activated more than once.
 *
 * @param self     an kosmonaut instance
 * @param callback callback function called when a message is received.
 * @return 0 if OK, otherwise appropriate error code
 */
int kosmonaut_listen(kosmonaut_t* self, kosmonaut_listener_t callback);

/**
 * Stops the listener.
 * If a listener's event loop is running, then stops it and cleans up
 * its status.
 *
 * @param self an kosmonaut instance
 */
void kosmonaut_stop_listening(kosmonaut_t* self);

#endif /* _KOSMONAUT_H_ */
