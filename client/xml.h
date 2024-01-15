#pragma once
#include "ast.h"

#define PATH_TO_RESPONSE_XSD "../response.xsd"

char* ast2xml(struct ast* root);

int validate_response(char* xml_content);

void readable_response(char *xml_content);