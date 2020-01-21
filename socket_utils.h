#ifndef SOCK_UTILS
#define SOCK_UTILS

#define BUFFER_SIZE     _IO_BUFSIZ 
#define ADDR_SIZE       256
#define INF_TRUE        1

#include <libaio.h>


enum socket_state {
    IDLE,
    RECEIVED,
    HEADER,
    STATIC_FILE,
    DYNAMIC_FILE,
    PHP_PROCESS_START,
    PHP_FINISHED_EXECUTION,
    PREP_ASYNC_READ,
    READ_DYNAMIC_CHUNK,
    SEND_DYNAMIC_CHUNK,
    FINISHED
};


enum fd_type {
    STATIC,
    DYNAMIC,
    NOT_FOUND
};


enum connection_state {
	STATE_DATA_RECEIVED,
	STATE_DATA_SENT,
	STATE_CONNECTION_CLOSED
};


enum http_status {
    HTTP200,
    HTTP404
};

enum request_method {
    GET_REQUEST,
    POST_REQUEST
};

struct as_connection {
    int conn_socket;
    char recv_buff[BUFFER_SIZE];
    size_t recv_size;
    size_t to_read;

    char request[BUFFER_SIZE];
    size_t req_size;

    char conn_addr[ADDR_SIZE];
    char resource_path[BUFFER_SIZE];
    char *received_body;
        
    int fd;
    size_t file_size;
    size_t sent_size;
    enum fd_type fdtype;
    
    enum http_status http_stat;
    enum socket_state sock_state;
	enum connection_state state;
    enum request_method req_method;

    int evfd;
    int sigfd;
    io_context_t ctx;
	struct iocb io;
};


int as_create_socketfd(int, int);
struct as_connection* as_connection_create(int);
void as_connection_handler(int);
void as_client_request_handler(struct as_connection*);
void as_client_response_handler(struct as_connection*);


static inline int min(int a, int b) {
    if (a > b)
        return b;
    return a;
}

#endif
