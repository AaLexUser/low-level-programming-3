#include "xml.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <string.h>
#include <stdbool.h>

void add_node(xmlNodePtr parent, struct ast *node)
{
    if (!node)
        return;

    xmlNodePtr xmlNode;
    char buffer[256];

    switch (node->nodetype)
    {
    case NT_RETURN:
    {
        struct return_ast *return_ast = (struct return_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "return", NULL);
        add_node(xmlNode, return_ast->value);
        break;
    }
    case NT_INSERT:
    {
        struct insert_ast *insert_ast = (struct insert_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "insert", NULL);
        snprintf(buffer, sizeof(buffer), "%s", insert_ast->tabname);
        xmlNewProp(xmlNode, BAD_CAST "tabname", BAD_CAST buffer);
        xmlNodePtr list = xmlNewChild(xmlNode, NULL, BAD_CAST "list", NULL);
        add_node(list, insert_ast->list);
        break;
    }
    case NT_UPDATE:
    {
        struct update_ast *update_ast = (struct update_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "update", NULL);
        snprintf(buffer, sizeof(buffer), "%s", update_ast->tabname);
        xmlNewProp(xmlNode, BAD_CAST "tabname", BAD_CAST buffer);
        add_node(xmlNode, update_ast->attr);
        xmlNodePtr list = xmlNewChild(xmlNode, NULL, BAD_CAST "list", NULL);
        add_node(list, update_ast->list);
        break;
    }
    case NT_REMOVE:
    {
        struct remove_ast *remove_ast = (struct remove_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "remove", NULL);
        snprintf(buffer, sizeof(buffer), "%s", remove_ast->tabname);
        xmlNewProp(xmlNode, BAD_CAST "tabname", BAD_CAST buffer);
        add_node(xmlNode, remove_ast->attr);
        break;
    }
    case NT_CREATE:
    {
        struct create_ast *create_ast = (struct create_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "create", NULL);
        snprintf(buffer, sizeof(buffer), "%s", create_ast->name);
        xmlNewProp(xmlNode, BAD_CAST "tabname", BAD_CAST buffer);
        xmlNodePtr list = xmlNewChild(xmlNode, NULL, BAD_CAST "list", NULL);
        add_node(list, create_ast->difinitions);
        break;
    }
    case NT_DROP:
    {
        struct drop_ast *drop_ast = (struct drop_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "drop", NULL);
        snprintf(buffer, sizeof(buffer), "%s", drop_ast->name);
        xmlNewProp(xmlNode, BAD_CAST "tabname", BAD_CAST buffer);
        break;
    }
    case NT_LIST:
    {
        struct list_ast *list_ast = (struct list_ast *)node;
        add_node(parent, list_ast->next);
        add_node(parent, list_ast->value);
        break;
    }
    case NT_PAIR:
    {
        struct pair_ast *pair_ast = (struct pair_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "pair", NULL);
        snprintf(buffer, sizeof(buffer), "%s", pair_ast->key);
        xmlNewProp(xmlNode, BAD_CAST "key", BAD_CAST buffer);
        add_node(xmlNode, pair_ast->value);
        break;
    }
    case NT_CREATE_PAIR:
    {
        struct create_pair_ast *create_pair_ast = (struct create_pair_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "definition", NULL);
        snprintf(buffer, sizeof(buffer), "%s", create_pair_ast->name);
        xmlNewProp(xmlNode, BAD_CAST "name", BAD_CAST buffer);
        snprintf(buffer, sizeof(buffer), "%s", str_type[create_pair_ast->type]);
        xmlNewProp(xmlNode, BAD_CAST "type", BAD_CAST buffer);
        break;
    }
    case NT_MERGE:
    {
        struct merge_ast *merge_ast = (struct merge_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "merge", NULL);
        snprintf(buffer, sizeof(buffer), "%s", merge_ast->var1);
        xmlNewProp(xmlNode, BAD_CAST "var1", BAD_CAST buffer);
        snprintf(buffer, sizeof(buffer), "%s", merge_ast->var2);
        xmlNewProp(xmlNode, BAD_CAST "var2", BAD_CAST buffer);
        break;
    }
    case NT_MERGE_PROJECTIONS:
    {
        struct merge_projections_ast *merge_projections_ast = (struct merge_projections_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "merge_projections", NULL);
        xmlNodePtr list = xmlNewChild(xmlNode, NULL, BAD_CAST "list", NULL);
        add_node(list, merge_projections_ast->list);
        break;
    }
    case NT_ATTR_NAME:
    {
        struct attr_name_ast *attr_name_ast = (struct attr_name_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "attr_name", NULL);
        snprintf(buffer, sizeof(buffer), "%s", attr_name_ast->variable);
        xmlNewProp(xmlNode, BAD_CAST "variable", BAD_CAST buffer);
        if (attr_name_ast->attr_name != NULL)
        {
            snprintf(buffer, sizeof(buffer), "%s", attr_name_ast->attr_name);
            xmlNewProp(xmlNode, BAD_CAST "attribute", BAD_CAST buffer);
        }
        break;
    }
    case NT_FILTER_CONDITION:
    {
        struct filter_condition_ast *filter_condition_ast = (struct filter_condition_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "conditions", NULL);
        if (filter_condition_ast->logic != -1)
        {
            snprintf(buffer, sizeof(buffer), "%s", str_cond[filter_condition_ast->logic]);
            xmlNewProp(xmlNode, BAD_CAST "logic", BAD_CAST buffer);
        }
        add_node(xmlNode, filter_condition_ast->l);
        add_node(xmlNode, filter_condition_ast->r);
        break;
    }
    case NT_FILTER_EXPR:
    {
        struct filter_expr_ast *filter_expr_ast = (struct filter_expr_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "filter_expr", NULL);
        const char *cmp_str = str_cond[filter_expr_ast->cmp];
        xmlNewProp(xmlNode, BAD_CAST "cmp", BAD_CAST str_cond[filter_expr_ast->cmp]);
        add_node(xmlNode, filter_expr_ast->attr_name);
        add_node(xmlNode, filter_expr_ast->constant);
        break;
    }
    case NT_FILTER:
    {
        struct filter_ast *filter_ast = (struct filter_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "filter", NULL);
        add_node(xmlNode, filter_ast->conditions_tree_root);
        break;
    }
    case NT_FOR:
    {
        struct for_ast *for_ast = (struct for_ast *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "for", NULL);
        snprintf(buffer, sizeof(buffer), "%s", for_ast->var);
        xmlNewProp(xmlNode, BAD_CAST "var", BAD_CAST buffer);
        snprintf(buffer, sizeof(buffer), "%s", for_ast->tabname);
        xmlNewProp(xmlNode, BAD_CAST "tabname", BAD_CAST buffer);
        if (for_ast->nonterm_list_head != NULL)
        {
            xmlNodePtr list = xmlNewChild(xmlNode, NULL, BAD_CAST "list", NULL);
            add_node(list, for_ast->nonterm_list_head);
        }
        if (for_ast->terminal != NULL)
        {
            add_node(xmlNode, for_ast->terminal);
        }
        break;
    }
    case NT_INTVAL:
    {
        struct nint *int_ast = (struct nint *)node;
        snprintf(buffer, sizeof(buffer), "%d", int_ast->value);
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "int", BAD_CAST buffer);
        break;
    }
    case NT_FLOATVAL:
    {
        struct nfloat *float_ast = (struct nfloat *)node;
        snprintf(buffer, sizeof(buffer), "%4.4f", float_ast->value);
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "float", BAD_CAST buffer);
        break;
    }
    case NT_STRINGVAL:
    {
        struct nstring *string_ast = (struct nstring *)node;
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "string", BAD_CAST string_ast->value);
        break;
    }
    case NT_BOOLVAL:
    {
        struct nint *bool_ast = (struct nint *)node;
        snprintf(buffer, sizeof(buffer), "%s", bool_ast->value ? "true" : "false");
        xmlNode = xmlNewChild(parent, NULL, BAD_CAST "bool", BAD_CAST buffer);
        break;
    }
    default:
    {
        fprintf(stderr, "unknown nodetype");
        return;
    }
    }
}

char *ast2xml(struct ast *root)
{
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root_element = xmlNewNode(NULL, BAD_CAST "root");
    xmlDocSetRootElement(doc, root_element);
    add_node(root_element, root);
    xmlChar *xmlbuff;
    int buffersize;
    xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
    // xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return (char *)xmlbuff;
}

int validate_response(char *xml_content)
{
    xmlDocPtr doc = xmlReadMemory(xml_content, strlen(xml_content), NULL, NULL, 0);
    if (doc == NULL)
    {
        fprintf(stderr, "Failed to parse document\n");
        return -1;
    }
    xmlSchemaParserCtxtPtr parserCtxt = xmlSchemaNewParserCtxt(PATH_TO_RESPONSE_XSD);
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

xmlNodePtr get_child(xmlNodePtr parent)
{
    if (parent == NULL)
        return NULL;
    xmlNodePtr child = parent->children;
    while (child != NULL)
    {
        if (child->type == XML_ELEMENT_NODE)
        {
            return child;
        }
        child = child->next;
    }
    return NULL;
}

xmlNodePtr get_next(xmlNodePtr node)
{
    if (node == NULL)
        return NULL;
    xmlNodePtr next = node->next;
    while (next != NULL)
    {
        if (next->type == XML_ELEMENT_NODE)
        {
            return next;
        }
        next = next->next;
    }
    return NULL;
}

void print_response(xmlDocPtr doc, xmlNodePtr cur)
{
    bool is_error = false;
    char* status = (char *)xmlGetProp(cur, BAD_CAST "status");
    if(status != NULL && strcmp(status, "OK") != 0){
        is_error = true;
        printf("\033[0;31m");
    }
    cur = get_child(cur);
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"message")))
        {
            printf("Message: %s\n", xmlNodeGetContent(cur));
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"table")))
        {
            printf("Table:\n");
            xmlNodePtr schema = get_child(cur);
            while (schema != NULL)
            {
                if ((!xmlStrcmp(schema->name, (const xmlChar *)"schema")))
                {
                    xmlNodePtr field = get_child(schema);
                    while (field != NULL)
                    {
                        if ((!xmlStrcmp(field->name, (const xmlChar *)"field")))
                        {
                            printf("%-15s\t", xmlGetProp(field, BAD_CAST "name"));
                        }
                        field = get_next(field);
                    }
                    printf("\n");
                }
                schema = get_next(schema);
            }
            xmlNodePtr rows = get_child(cur);
            while (rows != NULL)
            {
                if ((!xmlStrcmp(rows->name, (const xmlChar *)"rows")))
                {
                    xmlNodePtr row = get_child(rows);
                    while (row != NULL)
                    {
                        if ((!xmlStrcmp(row->name, (const xmlChar *)"row")))
                        {
                            xmlNodePtr element = get_child(row);
                            while (element != NULL)
                            {
                                if ((!xmlStrcmp(element->name, (const xmlChar *)"element")))
                                {
                                    printf("%-15s\t", (char* )xmlNodeGetContent(element));
                                }
                                element = get_next(element);
                            }
                            printf("\n");
                        }
                        row = get_next(row);
                    }
                }
                rows = get_next(rows);
            }
        }
        cur = get_next(cur);
    }
    if(is_error){
        printf("\033[0m");
    }
}

void readable_response(char *xml_content)
{
    xmlDocPtr doc = xmlReadMemory(xml_content, strlen(xml_content), NULL, NULL, 0);
    if (doc == NULL)
    {
        fprintf(stderr, "Failed to parse document\n");
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    print_response(doc, root);

    xmlFreeDoc(doc);
    xmlCleanupParser();
}