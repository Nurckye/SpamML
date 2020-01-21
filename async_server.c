// Author: Nitescu Radu
// Bucharest - 2020 January

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/epoll.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "constants.h"
#include "util.h"
#include "epoll_utils.h"
#include "socket_utils.h"
#include "http_parser.h"

extern struct server_options as_server_options;

http_parser request_parser;

static int server_socket;
int epollfd;

sigset_t mask;                                                             

static void as_close_cleanup(int signal_number)
{
    char s_sign[3];

    if (signal_number == SIGINT || signal_number == SIGQUIT) {
        sprintf(s_sign, "%d", signal_number);
        as_logger_arg("Cleaning up before closing. Got signal:", s_sign);
        close(server_socket);
        close(epollfd);
        exit(EXIT_SUCCESS);
    }
}


static void set_signals(void)
{
   int rc;                                                                    
   struct sigaction sa;                                                       
                                                                               
   sa.sa_flags = SA_SIGINFO;                                                  
   sa.sa_handler = as_close_cleanup;       

   sigemptyset(&mask);                                                        
   sa.sa_mask = mask;

   sigaddset(&mask, SIGCHLD);
   sigprocmask(SIG_BLOCK, &mask, NULL);                         

   rc = sigaction(SIGINT, &sa, NULL);
   DIE(rc == -1, "Setting signals error");          

   rc = sigaction(SIGQUIT, &sa, NULL);     
   DIE(rc == -1, "Setting signals error"); 

   signal(SIGPIPE, SIG_IGN);
}


int main(int argc, char *argv[])
{
    int server_socket;
    int rc;
    struct epoll_event res_ev;

    as_logger("Server started.") ;

    as_arg_parse(argc, argv);
    
    set_signals();
    

    server_socket = as_create_socketfd(as_server_options.port, 6);
    DIE(server_socket < 0, "Socket create error.\n");
    
    /**/
    epollfd = as_epoll_create();
    DIE(epollfd < 0, "Epoll create error.\n");

    rc = as_epoll_add_fd(server_socket, EPOLLIN);
    DIE(rc == -1, "Epoll add error.\n");

    while (INF_TRUE) {
        rc = as_epoll_wait_loop(&res_ev);
        DIE(rc == -1, "Epoll wait error.\n");

        if (res_ev.data.fd == server_socket) {
            as_connection_handler(server_socket);
        }
        else {
            if (res_ev.events & EPOLLIN) 
                as_client_request_handler(res_ev.data.ptr);
            
            if (res_ev.events & EPOLLOUT) 
                as_client_response_handler(res_ev.data.ptr);
        }
    }

    return 0;
}