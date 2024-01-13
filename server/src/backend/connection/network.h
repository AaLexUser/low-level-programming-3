/********************************
 * Author:      Alexey Lapin.   *
 * Description: Network module. *
 * Created:     13.01.2024      *
 * Last edit:   13.01.2024      *
 ********************************/

#ifndef NETWORK_H
#define NETWORK_H

int init_socket(int port);

int listen_socket(int sock);

int accept_socket(int sock);

int read_socket(int sock, char** message);

int write_socket(int sock, char *buffer, int size);

int close_socket(int sock);

#endif
