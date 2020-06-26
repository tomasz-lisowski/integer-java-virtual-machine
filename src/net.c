#include "net.h"


// Declarations of static functions
static inline uint32_t ref_to_index(const word_t net_ref);
static inline word_t index_to_ref(const uint32_t net_i);
static word_t socket_create(const uint32_t host, const uint16_t port);
static void net_check_ref(const word_t net_ref);
static word_t socket_store(const Sock_t* sock);


static const uint32_t k_index_to_ref = 0xCC00000C;
static const uint32_t k_ref_to_index = 0x00FFFFF0;

static MArr_t net_conn = {0, NULL, NULL}; // Keep track of connections


/**
* Recover network index from network reference
**/
static inline uint32_t ref_to_index(const word_t net_ref)
{
    return (k_ref_to_index & (uint32_t)net_ref) >> 4;
}


/**
* Create a network reference from network index
**/
static inline word_t index_to_ref(const uint32_t net_i)
{
    return (word_t)(k_index_to_ref | (net_i << 4));
}


/**
* Create an IPv4 TCP socket and return it's network reference
**/
static word_t socket_create(const uint32_t host, const uint16_t port)
{
    Sock_t* sock = (Sock_t*)malloc(sizeof(Sock_t));
    sock->fd = socket(AF_INET, SOCK_STREAM, 0);
    sock->fds = -1;
    sock->client = false;
    sock->server = false;
    sock->addr.sin_family = AF_INET;
    sock->addr.sin_port = htons(port);
    sock->addr.sin_addr.s_addr = htonl(host);
    sock->addr_len = sizeof(sock->addr);
    return socket_store(sock);
}


/**
* Utility to check if network reference is valid
**/
static void net_check_ref(const word_t net_ref)
{
    if ((((uint32_t)net_ref & 0xFF00000F) ^ k_index_to_ref) != 0)
    {
        fprintf(stderr, "[ERR] Invalid network reference or program requires more connections than is possible. In \"net.c::net_check_ref\".\n");
        destroy_ijvm_now();
    }
}


/**
* Add the socket to network connections so it can be tracked/modified.
* Return  network reference of saved socket
**/
static word_t socket_store(const Sock_t* sock)
{
    uint32_t net_i;
    word_t net_ref;

    // Initialize if needed
    if (net_conn.size == 0)
    {
        marr_init(&net_conn, NET_CONN_MIN_NUM);
    }

    // Save connection info
    net_i = marr_add_element(&net_conn, (uintptr_t)sock);
    if (net_i == SIZE_MAX_UINT32_T)
    {
        marr_resize(&net_conn, net_conn.size + 16);
        net_i = marr_add_element(&net_conn, (uintptr_t)sock);
    }

    // Return network reference
    net_ref = index_to_ref(net_i);
    net_check_ref(net_ref);
    return net_ref;
}


word_t net_bind(const word_t port)
{
    const word_t net_ref = socket_create(INADDR_ANY, (uint16_t)port);
    const uint32_t net_i = ref_to_index(net_ref);
    Sock_t* sock = (Sock_t*)marr_get_element(&net_conn, net_i);
    
    int success = bind(sock->fd, (struct sockaddr*)&sock->addr, sock->addr_len);
    if (success == -1)
    {
        return 0; // Failed to bind a socket
    }
    success = listen(sock->fd, NET_MAX_BACKLOG);
    if (success == -1)
    {
        return 0; // Failed to start listening for connection
    }

    /**
    * Primary socket is listening for connections.
    * Secondary socket is part of the connection.
    **/
    sock->fds = accept(sock->fd, (struct sockaddr*)&sock->addr, &sock->addr_len);
    if (sock->fds == -1)
    {
        return 0; // Failed to start accepting connections
    }

    sock->server = true;
    return net_ref;
}


word_t net_connect(const word_t host, const word_t port)
{
    const word_t net_ref = socket_create((uint32_t)host, (uint16_t)port);
    const uint32_t net_i = ref_to_index(net_ref);
    Sock_t* sock = (Sock_t*)marr_get_element(&net_conn, net_i);

    const int success = connect(sock->fd, (struct sockaddr*)&sock->addr, sock->addr_len);
    if (success == -1)
    {
        return 0; // Failed to connect
    }

    sock->client = true;
    return net_ref;
}


char net_recv(const word_t net_ref)
{
    uint32_t net_i;
    Sock_t* sock;
    ssize_t bytes_recvd = 0;
    char data_buffer = 0;

    net_check_ref(net_ref);
    net_i = ref_to_index(net_ref);
    sock = (Sock_t*)marr_get_element(&net_conn, net_i);

    // Receive data
    if (sock->client == true && sock->server == false)
    {
        bytes_recvd = recv(sock->fd, &data_buffer, 1, 0);
    }
    else if (sock->client == false && sock->server == true)
    {
        bytes_recvd = recv(sock->fds, &data_buffer, 1, 0);
    }
    else
    {
        fprintf(stderr, "[ERR] Socket cannot be both connected and bound. In \"net.c::net_recv\".\n");
        destroy_ijvm_now();
    }

    // Verify read
    if (bytes_recvd != 1)
    {
        fprintf(stderr, "[ERR] Received unexpected number of bytes. In \"net.c::net_recv\".\n");
        destroy_ijvm_now();
    }
    return data_buffer;
}


void net_send(const word_t net_ref, const word_t data)
{
    uint32_t net_i;
    Sock_t* sock;
    ssize_t bytes_sent = 0;

    net_check_ref(net_ref);
    net_i = ref_to_index(net_ref);
    sock = (Sock_t*)marr_get_element(&net_conn, net_i);

    //Send data
    if (sock->client == true && sock->server == false)
    {
        bytes_sent = send(sock->fd, &data, 4, 0);
    }
    else if (sock->client == false && sock->server == true)
    {
        bytes_sent = send(sock->fds, &data, 4, 0);
    }
    else
    {
        fprintf(stderr, "[ERR] Socket cannot be both connected and bound. In \"net.c::net_send\".\n");
        destroy_ijvm_now();
    }

    // Verify read
    if (bytes_sent != 4)
    {
        fprintf(stderr, "[ERR] Sent unexpected number of bytes. In \"net.c::net_send\".\n");
        destroy_ijvm_now();
    }
}


void net_close(const word_t net_ref)
{
    uint32_t net_i;
    Sock_t* sock;

    net_check_ref(net_ref);
    net_i = ref_to_index(net_ref);
    sock = (Sock_t*)marr_get_element(&net_conn, net_i);
    if (sock->client == true)
    {
        shutdown(sock->fd, 2);
        close(sock->fd);
        sock->client = false;
    }
    if (sock->server == true)
    {
        shutdown(sock->fd, 2);
        close(sock->fd);
        sock->server = false;
    }
    // If closing sockets failed, VM can't do anything about it

    free((Sock_t*)marr_get_element(&net_conn, net_i));
    marr_remove_element(&net_conn, net_i);
}


void net_destroy(void)
{
    for (uint32_t net_i = 0; net_i < net_conn.size; net_i++)
    {
        if (marr_check_marked(&net_conn, net_i) == true)
        {
            net_close(index_to_ref(net_i));
        }
    }
    marr_destroy(&net_conn);
}


void net_print(const bool compact)
{
    if (compact)
    {
        dprintf("NR[");
        for (uint32_t i = 0; i < net_conn.size; i++)
        {
            if (marr_check_marked(&net_conn, i) == true)
            {
                dprintf(" 0x%X", index_to_ref(i));
            }
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("NR\n");
        for (uint32_t i = 0; i < net_conn.size; i++)
        {
            if (marr_check_marked(&net_conn, i) == true)
            {
                dprintf("\t0x%X\n", index_to_ref(i));
            }
        }
    }
}