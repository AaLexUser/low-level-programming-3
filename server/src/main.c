#include <network/network.h>
#include <stdbool.h>
#include <pthread.h>
#include "backend/connection/xml.h"
#include "backend/connection/network.h"
#include "backend/db/db.h"

#define DEFAULT_FILE "main.db"
#define DEFAULT_PORT 8080
#define DEFAULT_BUFFER_SIZE 4096


void client_handler(const int* client)
{

    bool receiving_data = true;

    while (receiving_data) {
       char* message;
        if (read_socket(*client, &message) < 0) {
            receiving_data = false;
            break;
        }
        printf("Received: %s\n", message);
        struct ast* root = parse_xml_to_ast(message);
        print_ast(stdout, root,0);
        free_ast(root);
        free(message);
    }
    close_socket(*client);
}

int main(int argc, char** argv)
{
    char* filename = DEFAULT_FILE;
    int port = DEFAULT_PORT;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    if (argc > 2) {
        filename = argv[2];
    }


    db_init(filename);

    int sock = init_socket(port);
    if (sock < 0) {
        return 1;
    }

    if (listen_socket(sock) < 0) {
        return 1;
    }

    bool accepting_connections = true;
    while (accepting_connections) {
        int client = accept_socket(sock);
        if (client < 0) {
            return 1;
        }

        int* client_ptr = malloc(sizeof(int));
        *client_ptr = client;


        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, (void*)client_handler, client_ptr) != 0) {
            perror("Failed to create thread");
            accepting_connections = false;
            return 1;
        }
        pthread_detach(client_thread);
    }
    close_socket(sock);

    return 0;
}