#ifndef NET_H
#define NET_H


#include <sys/socket.h> // socket
#include <arpa/inet.h> // htons, htonl
#include <unistd.h> // read


#include "types.h"
#include "cpu.h"
#include "util.h"
#include "terminate.h"
#include "marr.h"


typedef struct Sock_t
{
    int fd; // File descriptor
    int fds; // Secondary file descriptor (for e.g. server sockets)
    bool client;
    bool server;
    struct sockaddr_in addr;
    socklen_t addr_len;
}Sock_t;


/**
* Create a new socket and bind it to a given port.
* Return  network reference on success
*         0 on failure
**/
word_t net_bind(const word_t port);


/**
* Create a new socket and connect it to a host at some port.
* Return  network reference on success
*         0 on failure
**/
word_t net_connect(const word_t host, const word_t port);


/**
* Receive one character on a socket with a given network reference.
* Return  received character
**/
char net_recv(const word_t net_ref);


/**
* Send one character on a socket with a given network reference
**/
void net_send(const word_t net_ref, const word_t data);


/**
* Close and delete a socket.
**/
void net_close(const word_t net_ref);


/**
* Close all sockets and free all associated memory
**/
void net_destroy(void);


/**
* Print out all active network references
**/
void net_print(const bool compact);


#endif
