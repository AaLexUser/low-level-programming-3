/**
 * @file network.c
 * @brief This file contains functions for network communication.
 */

#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Initializes a network socket.
 * 
 * This function creates a socket using the AF_INET (IPv4) address family,
 * SOCK_STREAM (TCP) socket type, and 0 as the protocol.
 * 
 * @return The socket file descriptor on success, or -1 on failure.
 */
int init_socket()
{
   int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Error creating socket");
        return -1;
    }
    return sock;
}

/**
 * @brief Connects to a server.
 * 
 * This function connects the socket to the specified server IP address and port.
 * 
 * @param sock The socket file descriptor.
 * @param ip The server IP address.
 * @param port The server port.
 * @return 0 on success, or -1 on failure.
 */
int connect_to_server(int sock, char* ip, int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
    {
        perror("Error converting ip address");
        return -1;
    }
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("Error connecting to server");
        return -1;
    }
    return 0;
}

/**
 * @brief Sends a message to the server.
 * 
 * This function sends a message to the server over the established connection.
 * 
 * @param sock The socket file descriptor.
 * @param message The message to send.
 * @return 0 on success, or -1 on failure.
 */
int send_message(int sock, char* message)
{
    int len = strlen(message);
    if (send(sock, &len, sizeof(len), 0) < 0)
    {
        perror("Error sending message length");
        return -1;
    }
    if (send(sock, message, len, 0) < 0)
    {
        perror("Error sending message");
        return -1;
    }
    return 0;
}

/**
 * @brief Receives a message from the server.
 * 
 * This function receives a message from the server over the established connection.
 * The received message is stored in the provided message pointer.
 * 
 * @param sock The socket file descriptor.
 * @param message A pointer to store the received message.
 * @return 0 on success, or -1 on failure.
 */
int receive_message(int sock, char** message)
{
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
    return 0;
}

/**
 * @brief Closes the network socket.
 * 
 * This function closes the network socket.
 * 
 * @param sock The socket file descriptor.
 * @return 0 on success, or -1 on failure.
 */
int close_socket(int sock)
{
    if (close(sock) < 0)
    {
        perror("Error closing socket");
        return -1;
    }
    return 0;
}

/**
 * @brief Closes the network connection.
 * 
 * This function gracefully closes the network connection by shutting down
 * both the reading and writing sides of the socket.
 * 
 * @param sock The socket file descriptor.
 * @return 0 on success, or -1 on failure.
 */
int close_connection(int sock)
{
    if (shutdown(sock, SHUT_RDWR) < 0)
    {
        perror("Error closing connection");
        return -1;
    }
    return 0;
}