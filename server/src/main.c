#include <network/network.h>
#include <stdbool.h>
#include <pthread.h>
#include "backend/connection/xml.h"
#include "backend/connection/network.h"
#include "backend/db/db.h"
#include "backend/connection/reqexe.h"
#include <signal.h>

#define DEFAULT_FILE "main.db"
#define DEFAULT_PORT 8080
#define DEFAULT_BUFFER_SIZE 4096

struct handler_args {
    db_t *db;
    int client;
};

volatile sig_atomic_t server_running = 1;
void sigint_handler(int sig) {
    server_running = 0;
}


void client_handler(struct handler_args *args) {

    bool receiving_data = true;

    while (receiving_data) {
        char *message;
        if (read_socket(args->client, &message) < 0) {
            receiving_data = false;
            continue;
        }
        if(strcmp(message, "") == 0){
            receiving_data = false;
            continue;
        }
        printf("Received: %s\n", message);
        struct ast *root = parse_xml_to_ast(message);
        print_ast(stdout, root, 0);
        struct response* resp  = reqexe(args->db, root);
        char* response_xml = response2xml(args->db, resp);
        printf("Sending: %s\n", response_xml);
        if (write_socket(args->client, response_xml, sizeof(response_xml)) < 0) {
            receiving_data = false;
            continue;
        }
        free_ast(root);
        free(message);
        xmlFree(response_xml);
    }
    close_socket(args->client);
    logger(LL_INFO, __func__, "Client %d disconnected", args->client);
}

int main(int argc, char **argv) {
    char *filename = DEFAULT_FILE;
    int port = DEFAULT_PORT;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    if (argc > 2) {
        filename = argv[2];
    }


    db_t *db = db_init(filename);

    int sock = init_socket(port);
    if (sock < 0) {
        return 1;
    }

    if (listen_socket(sock) < 0) {
        return 1;
    }

    logger(LL_INFO, __func__, "Listening on port %d", port);

    signal(SIGTERM, sigint_handler);

    fd_set readfds;
    int max_sd;

    FD_ZERO(&readfds); // Clear the socket set
    FD_SET(sock, &readfds); // Add server socket to set
    max_sd = sock; // Initially, the server socket is the max

    while (server_running) {
        int client = accept_socket(sock);
        if (client < 0) {
            return 1;
        }
        logger(LL_INFO, __func__, "Client %d connected", client);

        struct handler_args *args = malloc(sizeof(struct handler_args));
        args->db = db;
        args->client = client;


        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, (void *) client_handler, args) != 0) {
            perror("Failed to create thread");
            server_running = false;
            return 1;
        }
        pthread_detach(client_thread);
    }
    close_socket(sock);
    db_close();
    return 0;
}