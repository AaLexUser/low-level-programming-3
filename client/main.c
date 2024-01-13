#include "network.h"
#include "xml.h"
#include "ast.h"
#include <libxml/parser.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define DEFAULT_BUFFER_SIZE 4096
extern FILE *yyin;
extern struct ast *parse_ast(void);

struct ast *parse_query(char *query)
{
    yyin = fmemopen(query, strlen(query), "r");
    struct ast *root = parse_ast();
    return root;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: %s <server_host> <server_port>\n", argv[0]);
        return -1;
    }
    char *host = argv[1];
    int port = atoi(argv[2]);

    int sock = init_socket();
    if (sock < 0)
    {
        perror("Error creating socket");
        return -1;
    }

    if (connect_to_server(sock, host, port) < 0)
    {
        perror("Error connecting to server");
        return -1;
    }

    bool sending_queries = true;
    while (sending_queries)
    {
        char buffer[DEFAULT_BUFFER_SIZE];
        printf("> ");
        scanf("%[^\n]%*c", buffer);

        if (strcmp(buffer, "exit") == 0)
        {
            sending_queries = false;
            break;
        }

        struct ast *ast = parse_query(buffer);

        char *xml = ast2xml(ast);
        printf("%s\n", xml);

        if (send_message(sock, xml) < 0)
        {
            perror("Error sending message");
            return -1;
        }
        xmlFree(xml);

    }

    close_connection(sock);

    close_socket(sock);
    return 0;
}