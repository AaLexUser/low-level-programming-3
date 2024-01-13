#ifndef XML_H
#define XML_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>

#include "ast.h"
struct ast* get_list(xmlNodePtr node);
struct ast *xml2ast(xmlNode *node);

struct ast *parse_xml_to_ast(const char *xml_content);
#endif