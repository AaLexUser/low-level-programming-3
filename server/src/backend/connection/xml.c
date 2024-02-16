#include "xml.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <string.h>
#include <stdbool.h>

xmlNodePtr get_child(xmlNodePtr parent) {
    if (parent == NULL) return NULL;
    xmlNodePtr child = parent->children;
    while (child != NULL) {
        if (child->type == XML_ELEMENT_NODE) {
            return child;
        }
        child = child->next;
    }
    return NULL;
}

xmlNodePtr get_next(xmlNodePtr node) {
    if (node == NULL) return NULL;
    xmlNodePtr next = node->next;
    while (next != NULL) {
        if (next->type == XML_ELEMENT_NODE) {
            return next;
        }
        next = next->next;
    }
    return NULL;
}


ntype_t str_to_ntype(const char *type_str) {
    if (!type_str)
        return -1;
    for (int i = NT_INTEGER; i <= NT_BOOLEAN; i++) {
        if (strcmp(type_str, str_type[i]) == 0)
            return (ntype_t) i;
    }
    return -1;
}

ntype_t str_to_cond(const char *cond_str) {
    if (!cond_str)
        return -1;
    for (int i = NT_EQ; i <= NT_IN; i++) {
        if (strcmp(cond_str, str_cond[i]) == 0)
            return (ntype_t) i;
    }
    return -1;
}

struct ast *get_list(xmlNodePtr node) {
    struct ast *list = NULL;
    node = get_child(node);
    xmlNodePtr next = node;
    list = newlist(xml2ast(node), NULL);
    next = get_next(next);
    while (next != NULL) {
        list = newlist(xml2ast(next), list);
        next = get_next(next);
    }
    return list;
}

struct ast *xml2ast(xmlNode *node) {
    struct ast *ast_node = NULL;
    if (!node)
        return NULL;
    if (!xmlStrcmp(node->name, BAD_CAST "return")) {
        struct ast *ast_value = xml2ast(get_child(node));
        ast_node = newreturn(ast_value);
    } else if (!xmlStrcmp(node->name, BAD_CAST "insert")) {
        char *tabname = (char *) xmlGetProp(node, BAD_CAST "tabname");
        struct ast *ast_list = xml2ast(get_child(node));
        ast_node = newinsert(tabname, ast_list);
    } else if (!xmlStrcmp(node->name, BAD_CAST "update")) {
        char *tabname = (char *) xmlGetProp(node, BAD_CAST "tabname");
        struct ast *ast_attr = xml2ast(get_child(node));
        struct ast *ast_list = xml2ast(get_next(get_child(node)));
        ast_node = newupdate(tabname, ast_attr, ast_list);
    } else if (!xmlStrcmp(node->name, BAD_CAST "remove")) {
        char *tabname = (char *) xmlGetProp(node, BAD_CAST "tabname");
        struct ast *ast_attr = xml2ast(get_child(node));
        ast_node = newremove(tabname, ast_attr);
    } else if (!xmlStrcmp(node->name, BAD_CAST "create")) {
        char *tabname = (char *) xmlGetProp(node, BAD_CAST "tabname");
        struct ast *ast_difinitions = get_list(get_child(node));
        ast_node = newcreate(tabname, ast_difinitions);
    } else if (!xmlStrcmp(node->name, BAD_CAST "drop")) {
        char *tabname = (char *) xmlGetProp(node, BAD_CAST "tabname");
        ast_node = newdrop(tabname);
    } else if (!xmlStrcmp(node->name, BAD_CAST "list")) {
        ast_node = get_list(node);
    } else if (!xmlStrcmp(node->name, BAD_CAST "definition")) {
        char *name = (char *) xmlGetProp(node, BAD_CAST "name");
        char *type = (char *) xmlGetProp(node, BAD_CAST "type");
        ast_node = newcreate_pair(name, str_to_ntype(type));
    } else if (!xmlStrcmp(node->name, BAD_CAST "pair")) {
        char *key = (char *) xmlGetProp(node, BAD_CAST "key");
        struct ast *ast_value = xml2ast(get_child(node));
        ast_node = newpair(key, ast_value);
    } else if (!xmlStrcmp(node->name, BAD_CAST "merge")) {
        char *var1 = (char *) xmlGetProp(node, BAD_CAST "var1");
        char *var2 = (char *) xmlGetProp(node, BAD_CAST "var2");
        ast_node = newmerge(var1, var2);
    } else if (!xmlStrcmp(node->name, BAD_CAST "merge_projections")) {
        struct ast *ast_list = xml2ast(get_child(node));
        ast_node = newmerge_projections(ast_list);
    } else if (!xmlStrcmp(node->name, BAD_CAST "attr_name")) {
        char *variable = (char *) xmlGetProp(node, BAD_CAST "variable");
        char *attribute = (char *) xmlGetProp(node, BAD_CAST "attribute");
        ast_node = newattr_name(variable, attribute);
    } else if (!xmlStrcmp(node->name, BAD_CAST "conditions")) {
        const char *logic = (const char *) xmlGetProp(node, BAD_CAST "logic");
        struct ast *ast_l = xml2ast(get_child(node));
        struct ast *ast_r = xml2ast(get_next(get_child(node)));
        ast_node = newfilter_condition(ast_l, ast_r, str_to_cond(logic));
    } else if (!xmlStrcmp(node->name, BAD_CAST "filter_expr")) {
        const char *cmp = (const char *) xmlGetProp(node, BAD_CAST "cmp");
        struct ast *ast_attr_name = xml2ast(get_child(node));
        struct ast *ast_constant = xml2ast(get_next(get_child(node)));
        ast_node = newfilter_expr(ast_attr_name, ast_constant, str_to_cond(cmp));
    } else if (!xmlStrcmp(node->name, BAD_CAST "filter")) {
        struct ast *ast_conditions_tree_root = xml2ast(get_child(node));
        ast_node = newfilter(ast_conditions_tree_root);
    } else if (!xmlStrcmp(node->name, BAD_CAST "for")) {
        char *var = (char *) xmlGetProp(node, BAD_CAST "var");
        char *tabname = (char *) xmlGetProp(node, BAD_CAST "tabname");
        xmlNodePtr child_node = get_child(node);
        struct ast *ast_nonterm_list_head = NULL;
        struct ast *terminal = NULL;
        if (child_node) {
            if (xmlStrcmp(child_node->name, BAD_CAST "list") == 0) {
                ast_nonterm_list_head = xml2ast(child_node);
                xmlNodePtr termtemp = get_next(get_child(node));
                if (termtemp != NULL) {
                    terminal = xml2ast(termtemp);
                }
            } else {
                terminal = xml2ast(child_node);
            }
        }

        ast_node = newfor(var, tabname, ast_nonterm_list_head, terminal);
    } else if (!xmlStrcmp(node->name, BAD_CAST "int")) {
        int value = atoi((char *) xmlNodeGetContent(node->children));
        ast_node = newint(value);
    } else if (!xmlStrcmp(node->name, BAD_CAST "float")) {
        float value = atof((char *) xmlNodeGetContent(node->children));
        ast_node = newfloat(value);
    } else if (!xmlStrcmp(node->name, BAD_CAST "string")) {
        char *value = strdup((char *) xmlNodeGetContent(node->children));
        ast_node = newstring(value);
    } else if (!xmlStrcmp(node->name, BAD_CAST "bool")) {
        bool value = strcmp((char *) xmlNodeGetContent(node->children), "true") == 0;
        ast_node = newbool(value);
    } else if (!xmlStrcmp(node->name, BAD_CAST "list")) {
        struct ast *ast_value = xml2ast(get_child(node));
        struct ast *ast_next = xml2ast(get_next(get_child(node)));
        ast_node = newlist(ast_value, ast_next);
    }
    return ast_node;
}

struct ast *parse_xml_to_ast(const char *xml_content) {
    xmlDocPtr doc;
    xmlNodePtr root_element;

    doc = xmlReadMemory(xml_content, strlen(xml_content), NULL, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse document\n");
        return NULL;
    }

    root_element = xmlDocGetRootElement(doc);
    if (root_element == NULL) {
        fprintf(stderr, "Empty document\n");
        xmlFreeDoc(doc);
        return NULL;
    }

    xmlNodePtr first_child = get_child(root_element);

    struct ast *ast_root = xml2ast(first_child);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return ast_root;
}

char *response2xml(db_t *db, struct response *resp) {
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "response");
    xmlDocSetRootElement(doc, root_node);
    xmlNewProp(root_node, BAD_CAST "status", BAD_CAST (resp->status == 0 ? "OK" : "ERROR"));
    if (resp->message != NULL) {
        xmlNodePtr message_node = xmlNewChild(root_node, NULL, BAD_CAST "message", BAD_CAST resp->message);
    }
    if (resp->table != NULL) {
        xmlNodePtr table_node = xmlNewChild(root_node, NULL, BAD_CAST "table", NULL);
        xmlNewProp(table_node, BAD_CAST "name", BAD_CAST resp->table->name);
        table_t *table = resp->table;
        schema_t *schema = sch_load(table->schidx);
        xmlNodePtr schema_node = xmlNewChild(table_node, NULL, BAD_CAST "schema", NULL);
        sch_for_each(schema, chunk3, field3, chblix3, schema_index(schema)) {
            xmlNodePtr field_node = xmlNewChild(schema_node, NULL, BAD_CAST "field", NULL);
            xmlNewProp(field_node, BAD_CAST "name", BAD_CAST field3.name);
        }
        void *row = malloc(schema->slot_size);
        xmlNodePtr rows_node = xmlNewChild(table_node, NULL, BAD_CAST "rows", NULL);
        tab_for_each_row(table, chunk, chblix, row, schema) {
            xmlNodePtr row_node = xmlNewChild(rows_node, NULL, BAD_CAST "row", NULL);
            sch_for_each(schema, chunk2, field, chblix2, schema_index(schema)) {
                switch (field.type) {
                    case DT_INT: {
                        int64_t val = *(int64_t *) ((char *) row + field.offset);
                        char *value_str = malloc(21);
                        sprintf(value_str, "%"PRId64, val);
                        xmlNodePtr element_node = xmlNewChild(row_node, NULL, BAD_CAST "element", BAD_CAST value_str);
                        xmlNewProp(element_node, BAD_CAST "name", BAD_CAST field.name);
                        free(value_str);
                        break;
                    }
                    case DT_FLOAT: {
                        double val = *(double *) ((char *) row + field.offset);
                        char *value_str = malloc(21);
                        sprintf(value_str, "%0.2f", val);
                        xmlNodePtr element_node = xmlNewChild(row_node, NULL, BAD_CAST "element", BAD_CAST value_str);
                        xmlNewProp(element_node, BAD_CAST "name", BAD_CAST field.name);
                        free(value_str);
                        break;
                    }
                    case DT_VARCHAR: {
                        vch_ticket_t *vch = (vch_ticket_t *) ((char *) row + field.offset);
                        char *str = malloc(vch->size);
                        vch_get(db->varchar_mgr_idx, vch, str);
                        xmlNodePtr element_node = xmlNewChild(row_node, NULL, BAD_CAST "element", BAD_CAST str);
                        xmlNewProp(element_node, BAD_CAST "name", BAD_CAST field.name);
                        free(str);
                        break;
                    }
                    case DT_BOOL: {
                        bool val = *(bool *) ((char *) row + field.offset);
                        char *value_str = malloc(6);
                        sprintf(value_str, "%s", val ? "true" : "false");
                        xmlNodePtr element_node = xmlNewChild(row_node, NULL, BAD_CAST "element", BAD_CAST value_str);
                        xmlNewProp(element_node, BAD_CAST "name", BAD_CAST field.name);
                        free(value_str);
                        break;
                    }
                }
            }
        }
    }
    xmlChar *xmlbuff;
    int buffersize;
    xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
    xmlFreeDoc(doc);
    return (char *) xmlbuff;
}

int validate_request(char *xml_content) {
    xmlDocPtr doc = xmlReadMemory(xml_content, strlen(xml_content), NULL, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse document\n");
        return -1;
    }
    xmlSchemaParserCtxtPtr parserCtxt = xmlSchemaNewParserCtxt(PATH_TO_REQUEST_XSD);
    xmlSchemaPtr schema = xmlSchemaParse(parserCtxt);
    xmlSchemaValidCtxtPtr validCtxt = xmlSchemaNewValidCtxt(schema);
    int ret = xmlSchemaValidateDoc(validCtxt, doc);
    xmlSchemaFreeValidCtxt(validCtxt);
    xmlSchemaFree(schema);
    xmlSchemaFreeParserCtxt(parserCtxt);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return ret == 0;
}