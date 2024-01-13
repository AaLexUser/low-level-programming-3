#include "xml.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <string.h>
#include <stdbool.h>

xmlNodePtr get_child(xmlNodePtr parent){
    if(parent == NULL) return NULL;
    xmlNodePtr child = parent->children;
    while(child != NULL){
        if(child->type == XML_ELEMENT_NODE){
            return child;
        }
        child = child->next;
    }
    return NULL;
}

xmlNodePtr get_next(xmlNodePtr node){
    if(node == NULL) return NULL;
    xmlNodePtr next = node->next;
    while(next != NULL){
        if(next->type == XML_ELEMENT_NODE){
            return next;
        }
        next = next->next;
    }
    return NULL;
}

xmlNodePtr get_prev(xmlNodePtr node){
    xmlNodePtr prev = node->prev;
    while(prev != NULL){
        if(prev->type == XML_ELEMENT_NODE){
            return prev;
        }
        prev = prev->prev;
    }
    return NULL;
}

ntype_t str_to_ntype(const char *type_str)
{
    if (!type_str)
        return -1;
    for (int i = NT_INTEGER; i <= NT_BOOLEAN; i++)
    {
        if (strcmp(type_str, str_type[i]) == 0)
            return (ntype_t)i;
    }
    return -1;
}

ntype_t str_to_cond(const char *cond_str)
{
    if (!cond_str)
        return -1;
    for (int i = NT_EQ; i <= NT_IN; i++)
    {
        if (strcmp(cond_str, str_cond[i]) == 0)
            return (ntype_t)i;
    }
    return -1;
}

struct ast* get_list(xmlNodePtr node){
    struct ast* list = NULL;
    node = get_child(node);
    xmlNodePtr next = node;
    list = newlist(xml2ast(node), NULL);
    while(next != NULL){
        next = get_next(next);
        list = newlist(xml2ast(next), list);
    }
    return list;
}

struct ast *xml2ast(xmlNode *node)
{
    struct ast *ast_node = NULL;
    if (!node)
        return NULL;
    if (!xmlStrcmp(node->name, BAD_CAST "return"))
    {
        struct ast *ast_value = xml2ast(get_child(node));
        ast_node = newreturn(ast_value);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "insert"))
    {
        char *tabname = (char *)xmlGetProp(node, BAD_CAST "tabname");
        struct ast *ast_list = xml2ast(get_child(node));
        ast_node = newinsert(tabname, ast_list);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "update"))
    {
        char *tabname = (char *)xmlGetProp(node, BAD_CAST "tabname");
        struct ast *ast_attr = xml2ast(get_child(node));
        struct ast *ast_list = xml2ast(get_next(get_child(node)));
        ast_node = newupdate(tabname, ast_attr, ast_list);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "remove"))
    {
        char *tabname = (char *)xmlGetProp(node, BAD_CAST "tabname");
        struct ast *ast_attr = xml2ast(get_child(node));
        ast_node = newremove(tabname, ast_attr);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "create"))
    {
        char *tabname = (char *)xmlGetProp(node, BAD_CAST "tabname");
        struct ast *ast_difinitions = get_list(get_child(node));
        ast_node = newcreate(tabname, ast_difinitions);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "drop"))
    {
        char *tabname = (char *)xmlGetProp(node, BAD_CAST "tabname");
        ast_node = newdrop(tabname);
    }
    else if(!xmlStrcmp(node->name, BAD_CAST "list")){
        ast_node = get_list(node);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "definition"))
    {
        char *name = (char *)xmlGetProp(node, BAD_CAST "name");
        char *type = (char *)xmlGetProp(node, BAD_CAST "type");
        ast_node = newcreate_pair(name, str_to_ntype(type));
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "pair"))
    {
        char *key = (char *)xmlGetProp(node, BAD_CAST "key");
        struct ast *ast_value = xml2ast(get_child(node));
        ast_node = newpair(key, ast_value);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "merge"))
    {
        char *var1 = (char *)xmlGetProp(node, BAD_CAST "var1");
        char *var2 = (char *)xmlGetProp(node, BAD_CAST "var2");
        ast_node = newmerge(var1, var2);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "attr_name"))
    {
        char *variable = (char *)xmlGetProp(node, BAD_CAST "variable");
        char *attrubute = (char *)xmlGetProp(node, BAD_CAST "attrubute");
        ast_node = newattr_name(variable, attrubute);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "conditions"))
    {
        const char *logic = (const char *)xmlGetProp(node, BAD_CAST "logic");
        struct ast *ast_l = xml2ast(get_child(node));
        struct ast *ast_r = xml2ast(get_next(get_child(node)));
        ast_node = newfilter_condition(ast_l, ast_r, str_to_cond(logic));
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "filter_expr"))
    {
        const char *cmp = (const char *)xmlGetProp(node, BAD_CAST "cmp");
        struct ast *ast_attr_name = xml2ast(get_child(node));
        struct ast *ast_constant = xml2ast(get_next(get_child(node)));
        ast_node = newfilter_expr(ast_attr_name, ast_constant, str_to_cond(cmp));
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "filter"))
    {
        struct ast *ast_conditions_tree_root = xml2ast(get_child(node));
        ast_node = newfilter(ast_conditions_tree_root);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "for"))
    {
        char *var = (char *)xmlGetProp(node, BAD_CAST "var");
        char *tabname = (char *)xmlGetProp(node, BAD_CAST "tabname");
        struct ast *ast_nonterm_list_head = xml2ast(get_child(node));
        xmlNodePtr terminal = get_next(get_child(node));
        if(terminal != NULL)
            ast_node = newfor(var, tabname, ast_nonterm_list_head, xml2ast(terminal));
        else
            ast_node = newfor(var, tabname, ast_nonterm_list_head, NULL);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "int"))
    {
        int value = atoi((char *)xmlNodeGetContent(node->children));
        ast_node = newint(value);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "float"))
    {
        float value = atof((char *)xmlNodeGetContent(node->children));
        ast_node = newfloat(value);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "string"))
    {
        char *value = strdup((char *)xmlNodeGetContent(node->children));
        ast_node = newstring(value);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "bool"))
    {
        bool value = strcmp((char *)xmlNodeGetContent(node->children), "true") == 0;
        ast_node = newbool(value);
    }
    else if (!xmlStrcmp(node->name, BAD_CAST "list"))
    {
        struct ast *ast_value = xml2ast(get_child(node));
        struct ast *ast_next = xml2ast(get_next(get_child(node)));
        ast_node = newlist(ast_value, ast_next);
    }
    return ast_node;
}

struct ast *parse_xml_to_ast(const char *xml_content)
{
    xmlDocPtr doc;
    xmlNodePtr root_element;

    doc = xmlReadMemory(xml_content, strlen(xml_content), NULL, NULL, 0);
    if (doc == NULL)
    {
        fprintf(stderr, "Failed to parse document\n");
        return NULL;
    }

    root_element = xmlDocGetRootElement(doc);
    if (root_element == NULL)
    {
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