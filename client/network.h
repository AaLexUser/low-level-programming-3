#pragma once
#include <netinet/in.h>
#include <arpa/inet.h>

int init_socket();
int connect_to_server(int sock, char* ip, int port);
int send_message(int sock, char* message);
int receive_message(int sock, char** message);
int close_socket(int sock);
int close_connection(int sock);
