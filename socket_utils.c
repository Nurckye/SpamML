#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/eventfd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <libaio.h>
#include <libgen.h>
#include <errno.h>

#include "constants.h"
#include "util.h"
#include "socket_utils.h"
#include "epoll_utils.h"
#include "http_parser.h"


extern http_parser request_parser;
extern struct server_options as_server_options;

/* HTTP RESPONSE HEADER TEMPLATES */
const char HTTP404_HEADER[] = "HTTP/1.1 404 Not Found\r\n"
                              "Connection: close\r\n"
                              "Server: AsyncServer (Unix)\r\n\r\n";

const char HTTP200_HEADER[] = "HTTP/1.1 200 OK\r\n"
                              "Server: AsyncServer (Unix)\r\n"
                              "%s\r\n\r\n";


static void as_set_nonblock(int fd)
{
	int rc;
	int flags;

	rc = fcntl(fd, F_GETFL, 0);
	DIE(rc == -1, "Getting file descriptor flags.\n");

	flags = rc | O_NONBLOCK;

	rc = fcntl(fd, F_SETFL, flags);
	DIE(rc == -1, "Setting new file descriptor flags.\n");
}


static int on_path_cb(http_parser *p, const char *buf, size_t len)
{
    struct as_connection *conn = p->data;
	assert(p == &request_parser);
    
    memcpy(conn->resource_path, buf, len);
    conn->resource_path[len] = '\0';

	return 0;
}


static int on_body_cb(http_parser *p, const char *buf, size_t len)
{
    struct as_connection *conn = p->data;
	assert(p == &request_parser);
    
    conn->received_body = malloc((len + 1) * sizeof(*conn->received_body));
    DIE(conn->received_body == NULL, "Memory allocation error.");

    memcpy(conn->received_body, buf, len);
    conn->received_body[len] = '\0';

	return 0;
}


static http_parser_settings settings_parser= {
	/* on_message_begin */ 0,
	/* on_header_field */ 0,
	/* on_header_value */ 0,
	/* on_path */ on_path_cb,
	/* on_url */ 0,
	/* on_fragment */ 0,
	/* on_query_string */ 0,
	/* on_body */ 0,
	/* on_headers_complete */ on_body_cb,
	/* on_message_complete */ 0
};



int as_create_socketfd(int port, int backlog)
{
    int server_socket, rc, sock_opt;
    struct sockaddr_in server_address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    DIE(server_socket < 0, "Socket creation error.\n");

    sock_opt = 1;
	rc = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
				    &sock_opt, sizeof(int));
	DIE(rc < 0, "Set sock opt error.\n");
    
	memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;


    rc = bind(server_socket, 
              (struct sockaddr *)&server_address,
              sizeof(server_address));
    DIE(rc == -1, "Bind error.\n");

    rc = listen(server_socket, backlog);
    DIE(rc == -1, "Listen error.\n");

    return server_socket;
}


struct as_connection *as_connection_create(int sockfd)
{
    struct as_connection* new_conn = malloc(sizeof(*new_conn));
    
    new_conn->conn_socket = sockfd;
    as_set_nonblock(new_conn->conn_socket);
    
    new_conn->sock_state = IDLE;
    new_conn->sent_size = 0;
    new_conn->fd = UNINIT_FD;

    memset(&new_conn->io, 0, sizeof(new_conn->io));
    memset(new_conn->conn_addr, 0, ADDR_SIZE);
    memset(new_conn->resource_path, 0, BUFFER_SIZE);
    memset(new_conn->recv_buff, 0, BUFFER_SIZE);
    memset(new_conn->request, 0, BUFFER_SIZE);
    new_conn->received_body = NULL;

    return new_conn;
}


void as_connection_handler(int server_socket)
{
    int client_socket, rc;
    struct sockaddr_in addr;
	struct as_connection *conn;
	socklen_t addrlen = sizeof(struct sockaddr_in);


    client_socket = accept(server_socket, (struct sockaddr *)&addr, &addrlen);
	DIE(client_socket < 0, "Error in accept.\n");

    as_logger_arg("New connection from", inet_ntoa(addr.sin_addr));

    conn = as_connection_create(client_socket);
    rc = as_epoll_add_ptr(client_socket, conn, EPOLLIN);
    DIE(rc < 0, "Epoll add error.\n");
}


static int as_get_peer_address(int sockfd, void *buff)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    if (getpeername(sockfd, (struct sockaddr*)&addr, &addr_len) < 0)
		return -1;

    sprintf(buff, "%s:%d", inet_ntoa(addr.sin_addr), 
            ntohs(addr.sin_port));
    
    return 0;
}


static void as_remove_conn(struct as_connection *conn)
{
    int rc;
    as_logger_arg("Connection closed for", conn->conn_addr);
    rc = as_epoll_del_ptr(conn->conn_socket, conn);
    DIE(rc < 0, "[Closing connection.]\n");
    
    if (conn->received_body != NULL)
        free(conn->received_body);

    close(conn->conn_socket);
    free(conn);
}

static char *as_format_content_type(struct as_connection *conn)
{
    const char *ext = get_filename_extension(conn->resource_path);
    char *res = malloc(30 * sizeof(*res));
    int i;
    const char *images_format[5] = {"ico", "jpg", "png", "jpeg", "bmp"};
    const char *javascript_format[1] = {"js"};
    const char *style_format[1] = {"css"};
    const char *video_format[2] = {"mp4", "mpeg"};
    const char *audio_format[2] = {"vorbis", "wav"};
    const char *text_formats[7] = {"html", "php", "txt", "c", "cpp", "py", "md"};
    
    for (i = 0; i < 5; ++i) 
        if (! strcmp(ext, images_format[i])) {
            sprintf(res, "Content-Type: image/%s", ext);
            return res;
        }
    
    
    if (! strcmp(ext, javascript_format[0])) {
        strcpy(res, "Content-Type: text/javascript");
        return res;
    }

    if (! strcmp(ext, style_format[0])) {
        strcpy(res, "Content-Type: text/css");
        return res;
    }

    for (i = 0; i < 7; ++i) 
        if (! strcmp(ext, text_formats[i])) {
            strcpy(res, "Content-Type: text/html; charset=utf-8");
            return res;
        }

    for (i = 0; i < 2; ++i) 
        if (! strcmp(ext, video_format[i])) {
            sprintf(res, "Content-Type: video/%s", ext);
            return res;
        }

    
    if (! strcmp(ext, audio_format[i])) {
        sprintf(res, "Content-Type: audio/%s", ext);
        return res;
    }

    strcpy(res, "Content-Type: application/octet-stream");
    return res;
}

static void as_send_header(struct as_connection *conn)
{
    ssize_t recv_bytes;
    char res_header[BUFFER_SIZE];
    char *aux;
    memset(res_header, 0, BUFFER_SIZE);

    if (conn->http_stat == HTTP404) {
        as_logger_arg("Sent HTTP 404 to", conn->conn_addr);
        recv_bytes = send(conn->conn_socket, HTTP404_HEADER, strlen(HTTP404_HEADER), 0);
        if (recv_bytes < 0) {
            as_logger("Error in sending HTTP 404.");
            conn->sock_state = FINISHED;
        }
        if (recv_bytes == 0) {
            as_logger("Connection closed.");
            conn->sock_state = FINISHED;
        }
        conn->sock_state = FINISHED;
    }
    else if (conn->http_stat == HTTP200) {
        as_logger_arg("Sent HTTP 200 to", conn->conn_addr);
        aux = as_format_content_type(conn);
        if (conn->fdtype == DYNAMIC) 
            sprintf(res_header, HTTP200_HEADER, aux);
        else
            sprintf(res_header, HTTP200_HEADER, aux);
        free(aux);

        recv_bytes = send(conn->conn_socket, res_header, strlen(res_header), 0);
        if (recv_bytes < 0) {
            as_logger("Error in sending HTTP 200.");
            conn->sock_state = FINISHED;
        }
        if (recv_bytes == 0) {
            as_logger("Connection closed.");
            conn->sock_state = FINISHED;
        }
    }
}


static void as_php_render(struct as_connection *conn)
{
    int rc;
    pid_t pid;
    int filedes[2];
    
    const char *argv[] = {"php", conn->resource_path, conn->received_body, NULL};

    rc = pipe(filedes);
    DIE(rc < 0, "Pipe error\n");

    as_logger("Executing php file");
    pid = fork();
    // as_set_nonblock(filedes[STDOUT_FILENO]);
    // as_set_nonblock(filedes[STDIN_FILENO]);
    switch (pid)
    {
    case -1:
        DIE(1, "Fork error.\n");
        break;

    case 0: /* Child process */
        close(filedes[STDIN_FILENO]);
        dup2(filedes[STDOUT_FILENO], STDOUT_FILENO);
        execvp("php", (char *const *) argv);
        DIE(1, "Execvp error\n");
        break;

    default: /* Parent process */
        close(filedes[STDOUT_FILENO]);
        conn->fd = filedes[STDIN_FILENO];
        conn->sock_state = PHP_FINISHED_EXECUTION;
        conn->file_size = -1;
        rc = as_epoll_add_ptr(conn->fd, conn, EPOLLIN);
        DIE(rc < 0, "Epoll add error.\n");

        rc = as_epoll_del_ptr(conn->conn_socket, conn);
        DIE(rc < 0, "Epoll delete error.\n");

        break;
    }
}


static void as_prepare_file(struct as_connection *conn)
{
    int rc;
    char file_abs_path[BUFFER_SIZE];
    struct stat st;

    strcpy(file_abs_path, AWS_DOCUMENT_ROOT);
    if (as_server_options.target_directory != NULL) {
        strcat(file_abs_path, as_server_options.target_directory);
        strcat(file_abs_path, "/");
    }
        
    strcat(file_abs_path, conn->resource_path);


    rc = stat(file_abs_path, &st);
    if (rc == -1 || as_directory_exists(file_abs_path)) {
        conn->fdtype = NOT_FOUND;
        conn->http_stat = HTTP404;
        return;
    }
    
    DIE(realpath(file_abs_path, conn->resource_path) == NULL, 
        "Resolving path error.");


    if (as_server_options.typeof_content == STATIC_CONTENT)
        conn->fdtype = STATIC;
    
    else if (as_server_options.typeof_content == DYNAMIC_CONTENT)
        conn->fdtype = DYNAMIC;

    else {
        if (! strcmp("php", get_filename_extension(file_abs_path)))
            conn->fdtype = DYNAMIC;
        else 
            conn->fdtype = STATIC;
    }

    // printf("%s", get_filename_extension(file_abs_path));
    
    if (conn->fdtype == STATIC) {
        conn->fd = open(file_abs_path, O_RDONLY);
        conn->file_size = st.st_size;
        conn->http_stat = HTTP200;
    }

    else if (conn->fdtype == DYNAMIC) {
        as_php_render(conn);
        conn->http_stat = HTTP200;
    }
}


static int as_get_request_util(const char* s1, const char* s2)
{
    int i;
    for (i = 0; s1[i] != '\0' && s2[i] != '\0'; ++i) 
        if (s1[i] != s2[i])
            return 0;

    return 1;
}


static enum request_method as_get_request_method(const char* request) 
{
    if (as_get_request_util(request, "GET"))
        return GET_REQUEST;
    
    if (as_get_request_util(request, "POST"))
        return POST_REQUEST;
        
    return GET_REQUEST;
}


static enum connection_state as_receive_message(struct as_connection *conn)
{
    int rc;
    ssize_t recv_bytes;

    rc = as_get_peer_address(conn->conn_socket, conn->conn_addr);
    if (rc < 0) {
        as_logger("Error getting peer address.");
        goto err_remove_conn;
    }

    recv_bytes = recv(conn->conn_socket, conn->request, BUFFER_SIZE, 0);
    if (recv_bytes < 0) {
        as_logger_arg("Error in communication with", conn->conn_addr);
        goto err_remove_conn;
    }
    if (recv_bytes == 0) {
        as_logger_arg("Connection closed with", conn->conn_addr);
        goto err_remove_conn;
    }

    conn->req_size = recv_bytes;
    conn->sock_state = HEADER;
    
    as_logger_arg("Message received from", conn->conn_addr);
    
	http_parser_init(&request_parser, HTTP_REQUEST);

    conn->req_method = as_get_request_method(conn->request);
    request_parser.data = conn;
    
	recv_bytes = http_parser_execute(&request_parser, &settings_parser, 
                                     (char*)conn->request, conn->req_size);

    as_logger_arg("Asked for resource", conn->resource_path);

    as_prepare_file(conn);

    return STATE_DATA_RECEIVED;

err_remove_conn:
    as_remove_conn(conn);
    return STATE_CONNECTION_CLOSED;
}


static void as_setup_async_io(struct as_connection *conn)
{
    int rc;
	struct iocb *piocb;
    conn->ctx = 0;
    // conn->recv_size = min(BUFFER_SIZE, conn->file_size - conn->sent_size);
    conn->recv_size = BUFFER_SIZE;

    io_prep_pread(&conn->io, conn->fd, conn->recv_buff, 
                  conn->recv_size, conn->sent_size);
    
    conn->evfd = eventfd(0, 0);
    io_set_eventfd(&conn->io, conn->evfd);
    piocb = &conn->io;
    rc = as_epoll_add_ptr(conn->evfd, conn, EPOLLIN);
    DIE(rc < 0, "Epoll add fd.\n");

    io_setup(1, &conn->ctx);
    io_submit(conn->ctx, 1, &piocb);
    conn->sock_state = READ_DYNAMIC_CHUNK;
}


static void as_read_dynamic_chunk(struct as_connection *conn)
{
    int rc;
	struct io_event event;
    
    rc = io_getevents(conn->ctx, 1, 1, &event, NULL);   
    DIE(rc == -1, "Get events.\n");

    
    conn->sock_state = SEND_DYNAMIC_CHUNK;
    io_destroy(conn->ctx);

    rc = as_epoll_del_ptr(conn->evfd, conn);
    DIE(rc == -1, "Remove from epoll.\n");

    close(conn->evfd);
}


static void as_send_dynamic_chunk(struct as_connection *conn)
{
	ssize_t bytes_recv, buff_size;

	buff_size = strlen((char*)conn->recv_buff);
    bytes_recv = send(conn->conn_socket, conn->recv_buff,
		              buff_size, 0);

	conn->sent_size += bytes_recv;
	if (bytes_recv <= 0) {
        conn->sock_state = FINISHED;
		return;
	} 
    
    memset(conn->recv_buff, 0, BUFFER_SIZE);
    conn->sock_state = PREP_ASYNC_READ;
}


void as_client_request_handler(struct as_connection *conn)
{
    int rc;
	enum connection_state ret_state;

    // printf("conn->sock_state: %d\n", conn->sock_state);
    if (conn->sock_state == READ_DYNAMIC_CHUNK) {
        as_read_dynamic_chunk(conn);
    }

    else if (conn->sock_state == PHP_FINISHED_EXECUTION) {
        conn->sock_state = HEADER;

        rc = as_epoll_add_ptr(conn->conn_socket, conn, EPOLLOUT);
        DIE(rc < 0, "Epoll add error.\n");

        return;
    }

    else if (conn->sock_state == IDLE) {
        ret_state = as_receive_message(conn);
        if (ret_state == STATE_CONNECTION_CLOSED)
            return;

        if (conn->sock_state != PHP_FINISHED_EXECUTION) {
            rc = as_epoll_update_ptr(conn->conn_socket, conn, EPOLLIN | EPOLLOUT);
            DIE(rc < 0, "Epoll update error.\n");
        }
    }
}


static void as_static_send(struct as_connection *conn)
{
    ssize_t bytes_sent;

    bytes_sent = sendfile(conn->conn_socket, conn->fd, NULL, conn->file_size - conn->sent_size);
    conn->sent_size += bytes_sent;

    if (conn->sent_size >= conn->file_size) {
        as_logger_arg("Finished file transfer to", conn->conn_addr);
        conn->sock_state = FINISHED;
        return;
    }

    if (bytes_sent < 0) {
        as_logger("Error in sending.\n");
        conn->sock_state = FINISHED;
    }
    else if (bytes_sent == 0) {
        as_logger("Connection closed.\n");
        conn->sock_state = FINISHED;
    }
}


void as_client_response_handler(struct as_connection *conn)
{    
    if (conn->sock_state == HEADER) {
        as_send_header(conn);
        
        if (conn->sock_state != FINISHED) {
            if (conn->fdtype == STATIC)
                conn->sock_state = STATIC_FILE;
            else if (conn->fdtype == DYNAMIC)
                conn->sock_state = PREP_ASYNC_READ;
        }
        return;
    }

    if (conn->sock_state == STATIC_FILE) 
        as_static_send(conn);

    if (conn->sock_state == PREP_ASYNC_READ)
        as_setup_async_io(conn);

    if (conn->sock_state == SEND_DYNAMIC_CHUNK)
        as_send_dynamic_chunk(conn);

    if (conn->sock_state == FINISHED) {
        if (conn->fd != UNINIT_FD)
            close(conn->fd);
        as_remove_conn(conn);
    }
}