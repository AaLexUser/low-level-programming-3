#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"

void test_init_network_socket()
{
    int sock = init_network_socket();
    if (sock >= 0)
    {
        printf("init_network_socket test passed\n");
        close_network_socket(sock);
    }
    else
    {
        printf("init_network_socket test failed\n");
    }
}

void test_connect_to_server()
{
    int sock = init_network_socket();
    if (sock >= 0)
    {
        if (connect_to_server(sock, "127.0.0.1", 8080) == 0)
        {
            printf("connect_to_server test passed\n");
            close_network_socket(sock);
        }
        else
        {
            printf("connect_to_server test failed\n");
        }
    }
    else
    {
        printf("connect_to_server test failed: socket creation failed\n");
    }
}

void test_send_message()
{
    int sock = init_network_socket();
    if (sock >= 0)
    {
        if (connect_to_server(sock, "127.0.0.1", 8080) == 0)
        {
            char* message = "Hello, server!";
            if (send_message(sock, message) == 0)
            {
                printf("send_message test passed\n");
                close_network_socket(sock);
            }
            else
            {
                printf("send_message test failed\n");
            }
        }
        else
        {
            printf("send_message test failed: connection failed\n");
        }
    }
    else
    {
        printf("send_message test failed: socket creation failed\n");
    }
}

void test_receive_message()
{
    int sock = init_network_socket();
    if (sock >= 0)
    {
        if (connect_to_server(sock, "127.0.0.1", 8080) == 0)
        {
            char* message;
            if (receive_message(sock, &message) == 0)
            {
                printf("receive_message test passed: Received message: %s\n", message);
                free(message);
                close_network_socket(sock);
            }
            else
            {
                printf("receive_message test failed\n");
            }
        }
        else
        {
            printf("receive_message test failed: connection failed\n");
        }
    }
    else
    {
        printf("receive_message test failed: socket creation failed\n");
    }
}

void test_close_network_connection()
{
    int sock = init_network_socket();
    if (sock >= 0)
    {
        if (connect_to_server(sock, "127.0.0.1", 8080) == 0)
        {
            if (close_network_connection(sock) == 0)
            {
                printf("close_network_connection test passed\n");
                close_network_socket(sock);
            }
            else
            {
                printf("close_network_connection test failed\n");
            }
        }
        else
        {
            printf("close_network_connection test failed: connection failed\n");
        }
    }
    else
    {
        printf("close_network_connection test failed: socket creation failed\n");
    }
}

int main()
{
    test_init_network_socket();
    test_connect_to_server();
    test_send_message();
    test_receive_message();
    test_close_network_connection();

    return 0;
}