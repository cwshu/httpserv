/** 
 * @file server_arch.h
 * @brief very basic server architecture framework, like fork-based multiprocess server.
 */

#ifndef __SERVER_ARCH_H__
#define __SERVER_ARCH_H__

#include "socket.h"
#include "utils.h"

/** @brief function pointer type of service implementation function. For example, telnet_service and http_service. */
typedef void (*OneConnectionService)(socketfd_t connection_socket, SocketAddr& connection_addr);

socketfd_t bind_and_listen_tcp_socket(SocketAddr& listen_addr);

/**
 *
 * helping closing client_socket, so service_function doesn't need close it.
 * wait at receive SIGCHLD, release child resource for multiprocess && concurrent server 
 */
void start_multiprocess_server(socketfd_t listen_socket, OneConnectionService service_function);

/* start_multiprocess_server sub functions */
void sigchid_waitfor_child(int sig);

#endif
