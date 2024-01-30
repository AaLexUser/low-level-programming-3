#include <network/network.h>
#include <stdbool.h>
#include <pthread.h>
#include "backend/connection/xml.h"
#include "backend/connection/network.h"
#include "backend/db/db.h"
#include "backend/connection/query_execute/reqexe.h"
#include "backend/table/table.h"
#include <signal.h>

#define DEFAULT_FILE "main.db"
#define DEFAULT_PORT 8080

struct handler_args {
    db_t *db;
    int client;
};

volatile sig_atomic_t server_running = 1;

void sigint_handler(int sig) {
    server_running = 0;
}

fd_set readfds;
int max_sd;


void client_handler(struct handler_args *args) {

    bool receiving_data = true;

    while (receiving_data) {
        struct response *resp = malloc(sizeof(struct response));
        char *message;
        if (read_socket(args->client, &message) < 0) {
            receiving_data = false;
            continue;
        }
        if (strcmp(message, "") == 0) {
            receiving_data = false;
            continue;
        }
        printf("Received: %s\n", message);
        if(!validate_request(message)){
            printf("Invalid request\n");
            resp->message = strdup("Invalid request");
            resp->status = -1;
        }
        else {
            struct ast *root = parse_xml_to_ast(message);
            print_ast(stdout, root, 0);
            reqexe(args->db, root, resp);
            free_ast(root);
        }
        char *response_xml = response2xml(args->db, resp);
        printf("Sending: %s\n", response_xml);
        if (write_socket(args->client, response_xml, (int)strlen(response_xml)) < 0) {
            receiving_data = false;
            continue;
        }
        if(resp->table !=NULL && strcmp(resp->table->name,"TEMP") == 0){
            tab_drop(args->db, resp->table);
        }
        free(message);
        xmlFree(response_xml);
    }
    FD_CLR(args->client, &readfds);
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
    signal(SIGINT, sigint_handler);
    signal(SIGKILL, sigint_handler);

    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    max_sd = sock;


    struct timeval timeout;
    while (server_running) {
        fd_set tmpfds = readfds;
        timeout.tv_sec = 1; // Set a timeout 1 second to allow periodic checks for server_running
        timeout.tv_usec = 0;

        int activity = select(max_sd + 1, &tmpfds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR)) {
            fprintf(stderr,"select error: %s", strerror(errno));
            break;
        }

        if (activity > 0) {
            // Check if the activity is on the server socket (new connection)
            if (FD_ISSET(sock, &tmpfds)) {
                int client = accept_socket(sock);
                if (client < 0) {
                    perror("accept failed");
                    continue;
                }

                logger(LL_INFO, __func__, "Client %d connected", client);

                // Add new socket to the array of sockets
                FD_SET(client, &readfds);
                if (client > max_sd) {
                    max_sd = client;
                }

                struct handler_args *args = malloc(sizeof(struct handler_args));
                args->db = db;
                args->client = client;

                pthread_t client_thread;
                if (pthread_create(&client_thread, NULL, (void *) client_handler, args) != 0) {
                    perror("Failed to create thread");
                    server_running = false;
                    continue;
                }
                pthread_detach(client_thread);
            }
        }
    }

    for (int i = 0; i <= max_sd; i++) {
        if (FD_ISSET(i, &readfds)) {
            if(i != -1){
                close_socket(i);
            }
        }
    }
    db_close();
    return 0;
}