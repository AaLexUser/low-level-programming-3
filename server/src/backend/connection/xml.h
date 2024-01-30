#ifndef XML_H
#define XML_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include "backend/connection/query_execute/reqexe.h"

#include "ast.h"

#define PATH_TO_REQUEST_XSD "/Users/aleksei/ITMO/LLP_Lab3/server/src/request.xsd"
struct ast* get_list(xmlNodePtr node);
struct ast *xml2ast(xmlNode *node);

struct ast *parse_xml_to_ast(const char *xml_content);

char* response2xml(db_t* db, struct response* resp);

int validate_request(char* xml_content);
#endif