/**
 * @file network.c
 * @brief Implementation of network-related functions.
 */
#include "network.h"
#include "utils/logger.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Initializes a socket with the specified port.
 *
 * This function creates a socket using the AF_INET domain and the SOCK_STREAM type.
 * It then binds the socket to the specified port.
 *
 * @param port The port number to bind the socket to.
 * @return The socket file descriptor on success, or -1 on failure.
 */
int init_socket(int port)
{
    logger(LL_DEBUG, __func__, "Initializing socket with port %d", port);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        logger(LL_ERROR, __func__, "Unable to create socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = 0;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        logger(LL_ERROR, __func__, "Unable to bind socket %s", strerror(errno));
        return -1;
    }

    return sock;
}

/**
 * @brief Listens for incoming connections on the specified socket.
 *
 * This function listens for incoming connections on the specified socket.
 *
 * @param sock The socket file descriptor to listen on.
 * @return 0 on success, or -1 on failure.
 */
int listen_socket(int sock)
{
    logger(LL_DEBUG, __func__, "Listening socket %d", sock);
    if (listen(sock, 1) < 0)
    {
        logger(LL_ERROR, __func__, "Unable to listen socket %s", strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * @brief Accepts an incoming connection on the specified socket.
 *
 * This function accepts an incoming connection on the specified socket.
 *
 * @param sock The socket file descriptor to accept the connection on.
 * @return The client socket file descriptor on success, or -1 on failure.
 */
int accept_socket(int sock)
{
    logger(LL_DEBUG, __func__, "Accepting socket %d", sock);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int client = accept(sock, (struct sockaddr *)&addr, &len);
    if (client < 0)
    {
        logger(LL_ERROR, __func__, "Unable to accept socket %s", strerror(errno));
        return -1;
    }
    return client;
}

/**
 * Reads a message from a socket.
 *
 * @param sock The socket descriptor.
 * @param message A pointer to a char pointer that will store the received message.
 * @return Returns 0 on success, -1 on error.
 */
int read_socket(int sock, char** message)
{
    logger(LL_DEBUG, __func__, "Reading socket %d", sock);
    int len;
    if (recv(sock, &len, sizeof(len), 0) < 0)
    {
        perror("Error receiving message length");
        return -1;
    }
    *message = (char*)malloc(len + 1);
    if (recv(sock, *message, len, 0) < 0)
    {
        perror("Error receiving message");
        return -1;
    }
    (*message)[len] = '\0';
}

/**
 * @brief Writes data from the buffer to the specified socket.
 *
 * This function writes data from the buffer to the specified socket.
 *
 * @param sock The socket file descriptor to write to.
 * @param buffer The buffer containing the data to write.
 * @param size The number of bytes to write.
 * @return The number of bytes written on success, or -1 on failure.
 */
int write_socket(int sock, char *buffer, int size)
{
    logger(LL_DEBUG, __func__, "Writing socket %d", sock);
    int written = (int)send(sock, buffer, size, 0);
    if (written < 0)
    {
        logger(LL_ERROR, __func__, "Unable to write socket %s", strerror(errno));
        return -1;
    }
    return written;
}

/**
 * @brief Closes the specified socket.
 *
 * This function closes the specified socket.
 *
 * @param sock The socket file descriptor to close.
 * @return 0 on success, or -1 on failure.
 */
int close_socket(int sock)
{
    logger(LL_DEBUG, __func__, "Closing socket %d", sock);
    if (close(sock) < 0)
    {
        logger(LL_ERROR, __func__, "Unable to close socket %s", strerror(errno));
        return -1;
    }
    return 0;
}

