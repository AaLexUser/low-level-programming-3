#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>


struct ast*
newast(ntype_t nodetype, struct ast* l, struct ast* r)
{
    struct ast* ast = malloc(sizeof(struct ast));
    if (!ast) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    ast->nodetype = nodetype;
    ast->l = l;
    ast->r = r;
    return ast;
}

struct ast*
newint(int value)
{
    struct nint* intast = malloc(sizeof(struct nint));
    if (!intast) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    intast->nodetype = NT_INTVAL;
    intast->value = value;
    return (struct ast*)intast;
}

struct ast*
newfloat(double value)
{
    struct nfloat* floatast = malloc(sizeof(struct nfloat));
    if (!floatast) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    floatast->nodetype = NT_FLOATVAL;
    floatast->value = value;
    return (struct ast*)floatast;
}

struct ast*
newstring(char* value)
{
    struct nstring* stringast = malloc(sizeof(struct nstring));
    if (!stringast) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    stringast->nodetype = NT_STRINGVAL;
    stringast->value = value;
    return (struct ast*)stringast;
}

struct ast*
newbool(int value)
{
    struct nint* boolast = malloc(sizeof(struct nint));
    if (!boolast) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    boolast->nodetype = NT_BOOLVAL;
    boolast->value = value;
    return (struct ast*)boolast;
}

/*---------------------------for ast -----------------------------*/
struct ast*
newfor(char* var, char* tabname, struct ast* nonterm_list_head, struct ast* terminal)
{
    struct for_ast* forast = malloc(sizeof(struct for_ast));
    if (!forast) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    forast->nodetype = NT_FOR;
    forast->var = var;
    forast->tabname = tabname;
    forast->nonterm_list_head = nonterm_list_head;
    forast->terminal = terminal;
    return (struct ast*)forast;
}

/*---------------------------list ast -----------------------------*/
struct ast*
newlist(struct ast* value, struct ast* next)
{
    struct list_ast* listast = malloc(sizeof(struct list_ast));
    if (!listast) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    listast->nodetype = NT_LIST;
    listast->value = value;
    listast->next = next;
    return (struct ast*)listast;
}

/*---------------------------filter ast -----------------------------*/
struct ast*
newfilter(struct ast* conditions_tree_root)
{
    struct filter_ast* filterast = malloc(sizeof(struct filter_ast));
    if (!filterast) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    filterast->nodetype = NT_FILTER;
    filterast->conditions_tree_root = conditions_tree_root;
    return (struct ast*)filterast;
}

/*---------------------------filter condition ast -----------------------------*/
struct ast*
newfilter_condition(struct ast* l, struct ast* r, int logic)
{
    struct filter_condition_ast* filter_cond = malloc(sizeof(struct filter_condition_ast));
    if (!filter_cond) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    filter_cond->nodetype = NT_FILTER_CONDITION;
    filter_cond->l = l;
    filter_cond->r = r;
    filter_cond->logic = logic;
    return (struct ast*)filter_cond;
}

/*---------------------------filter expr ast -----------------------------*/
struct ast*
newfilter_expr(struct ast* attr_name, struct ast* constant, int cmp)
{
    struct filter_expr_ast* filter_expr = malloc(sizeof(struct filter_expr_ast));
    if (!filter_expr) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    filter_expr->nodetype = NT_FILTER_EXPR;
    filter_expr->attr_name = attr_name;
    filter_expr->constant = constant;
    filter_expr->cmp = cmp;
    return (struct ast*)filter_expr;
}

/*---------------------------attr name ast -----------------------------*/
struct ast*
newattr_name(char* variable, char* attr_name)
{
    struct attr_name_ast* attrname = malloc(sizeof(struct attr_name_ast));
    if (!attrname) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    attrname->nodetype = NT_ATTR_NAME;
    attrname->variable = variable;
    attrname->attr_name = attr_name;
    return (struct ast*)attrname;
}

/*---------------------------return ast ------------------------------*/
struct ast*
newreturn(struct ast* value)
{
    struct return_ast* returnast = malloc(sizeof(struct return_ast));
    if (!returnast) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    returnast->nodetype = NT_RETURN;
    returnast->value = value;
    return (struct ast*)returnast;
}

/*---------------------------merge ast -----------------------------*/
struct ast*
newmerge(char* var1, char* var2)
{
    struct merge_ast* mergeast = malloc(sizeof(struct merge_ast));
    if (!mergeast) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    mergeast->nodetype = NT_MERGE;
    mergeast->var1 = var1;
    mergeast->var2 = var2;
    return (struct ast*)mergeast;
}


/*---------------------------pair ast -----------------------------*/

struct ast*
newpair(char* key, struct ast* value)
{
    struct pair_ast* pair = malloc(sizeof(struct pair_ast));
    if (!pair) {
       fprintf(stderr, "out of space");
        return NULL;
    }
    pair->nodetype = NT_PAIR;
    pair->key = key;
    pair->value = value;
    return (struct ast*)pair;
}

/*---------------------------insert ast -----------------------------*/
struct ast*
newinsert(char* tabname, struct ast* list)
{
    struct insert_ast* insert = malloc(sizeof(struct insert_ast));
    if (!insert) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    insert->nodetype = NT_INSERT;
    insert->tabname = tabname;
    insert->list = list;
    return (struct ast*)insert;
}

/*---------------------------update ast -----------------------------*/
struct ast*
newupdate(char* tabname, struct ast* attr, struct ast* list)
{
    struct update_ast* update = malloc(sizeof(struct update_ast));
    if (!update) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    update->nodetype = NT_UPDATE;
    update->tabname = tabname;
    update->attr = attr;
    update->list = list;
    return (struct ast*)update;
}

/*---------------------------remove ast -----------------------------*/
struct ast*
newremove(char* tabname, struct ast* attr)
{
    struct remove_ast* remove = malloc(sizeof(struct remove_ast));
    if (!remove) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    remove->nodetype = NT_REMOVE;
    remove->tabname = tabname;
    remove->attr = attr;
    return (struct ast*)remove;
}

/*---------------------------create pair ast -----------------------------*/
struct ast*
newcreate_pair(char* name, int type)
{
    struct create_pair_ast* create_pair = malloc(sizeof(struct create_pair_ast));
    if (!create_pair) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    create_pair->nodetype = NT_CREATE_PAIR;
    create_pair->name = name;
    create_pair->type = type;
    return (struct ast*)create_pair;
}

/*---------------------------create ast -----------------------------*/
struct ast*
newcreate(char* name, struct ast* difinitions)
{
    struct create_ast* create = malloc(sizeof(struct create_ast));
    if (!create) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    create->nodetype = NT_CREATE;
    create->name = name;
    create->difinitions = difinitions;
    return (struct ast*)create;
}

/*---------------------------drop ast -----------------------------*/
struct ast*
newdrop(char* name)
{
    struct drop_ast* drop = malloc(sizeof(struct drop_ast));
    if (!drop) {
        fprintf(stderr, "out of space");
        return NULL;
    }
    drop->nodetype = NT_DROP;
    drop->name = name;
    return (struct ast*)drop;
}


static void print_indent(FILE* stream, int level)
{
    char* indent = malloc(sizeof(char) * (level*2 + 1));
    memset(indent, ' ', level*2+1);
    indent[level*2] = '\0';
    fprintf(stream, "%s", indent);
    free(indent);

}

static void print_node(FILE* stream, int level, char* string, ...)
{
    if(!string || !stream || level < 0)
        return;
    va_list args;
    va_start(args, string);
    print_indent(stream, level);
    vfprintf(stream, string, args);
    va_end(args);
}

void print_ast(FILE* stream, struct ast* ast, int level)
{
    if (!ast) {
        return;
    }
    switch (ast->nodetype) {
        case NT_RETURN: {
            struct return_ast* returnast = (struct return_ast*)ast;
            print_node(stream, level, "return: {\n");
            print_ast(stream, returnast->value, level+1);
            print_node(stream, level, "}\n");
            break;
        }
        case NT_INSERT: {
            struct insert_ast* insertast = (struct insert_ast*)ast;
            print_node(stream, level, "insert: {\n");
            print_node(stream, level+1, "tabname: %s\n", insertast->tabname);
            print_node(stream, level+1, "data: [\n");
            print_ast(stream, insertast->list, level+1);
            print_node(stream, level+1, "]\n");
            print_node(stream, level, "}\n");
            break;
        }
        case NT_UPDATE: {
            struct update_ast* updateast = (struct update_ast*)ast;
            print_node(stream, level, "update: {\n");
            print_node(stream, level+1, "tabname: %s\n", updateast->tabname);
            print_ast(stream, updateast->attr, level+1);
            print_node(stream, level+1, "data: [\n");
            print_ast(stream, updateast->list, level+1);
            print_node(stream, level+1, "]\n");
            print_node(stream, level, "}\n");
            break;
        }
        case NT_REMOVE: {
            struct remove_ast* removeast = (struct remove_ast*)ast;
            print_node(stream, level, "remove: {\n");
            print_node(stream, level+1, "tabname: %s\n", removeast->tabname);
            print_ast(stream, removeast->attr, level+1);
            print_node(stream, level, "}\n");
            break;
        }
        case NT_CREATE: {
            struct create_ast* createast = (struct create_ast*)ast;
            print_node(stream, level, "create: {\n");
            print_node(stream, level+1, "tabname: %s\n", createast->name);
            print_node(stream, level+1, "data: [\n");
            print_ast(stream, createast->difinitions, level+1);
            print_node(stream, level+1, "]\n");
            print_node(stream, level, "}\n");
            break;
        }
        case NT_DROP: {
            struct drop_ast* dropast = (struct drop_ast*)ast;
            print_node(stream, level, "drop: {\n");
            print_node(stream, level+1, "tabname: %s\n", dropast->name);
            print_node(stream, level, "}\n");
            break;
        }
        case NT_LIST: {
            struct list_ast* listast = (struct list_ast*)ast;
            print_ast(stream, listast->next, level);
            print_ast(stream, listast->value, level+1);
            break;
        }
        case NT_PAIR : {
            struct pair_ast* pairast = (struct pair_ast*)ast;
            print_node(stream, level, "pair: {\n");
            print_node(stream, level+1, "key: %s\n", pairast->key);
            print_node(stream, level+1, "value: {\n");
            print_ast(stream, pairast->value, level+2);
            print_node(stream, level+1, "}\n");
            print_node(stream, level, "}\n");
            break;
        }
        case NT_CREATE_PAIR: {
            struct create_pair_ast* create_pairast = (struct create_pair_ast*)ast;
            print_node(stream, level, "definition: {\n");
            print_node(stream, level+1, "name: %s\n", create_pairast->name);
            print_node(stream, level+1, "type: %s\n", str_type[create_pairast->type]);
            print_node(stream, level, "}\n");
            break;
        }
        case NT_MERGE: {
            struct merge_ast* mergeast = (struct merge_ast*)ast;
            print_node(stream, level, "merge: {\n");
            print_node(stream, level+1, "variable: %s\n", mergeast->var1);
            print_node(stream, level+1, "variable: %s\n", mergeast->var2);
            print_node(stream, level, "}\n");
            break;
        }
        case NT_ATTR_NAME: {
            struct attr_name_ast* attrnameast = (struct attr_name_ast*)ast;
            print_node(stream, level, "attr_name: {\n");
            print_node(stream, level+1, "variable: %s\n", attrnameast->variable);
            if(attrnameast->attr_name != NULL)
                print_node(stream, level+1, "attrubute: %s\n", attrnameast->attr_name);
            print_node(stream, level, "}\n");
            break;
        }
        case NT_FILTER_CONDITION: {
            struct filter_condition_ast* cond_ast = (struct filter_condition_ast*)ast;
            print_node(stream, level, "conditions: {\n");
            if(cond_ast->logic != -1)
                print_node(stream, level+1, "logic: %s\n", str_cond[cond_ast->logic]);
            print_ast(stream, cond_ast->l, level+1);
            print_ast(stream, cond_ast->r, level+1);
            print_node(stream, level, "}\n");
            break;
        }
        case NT_FILTER_EXPR: {
            struct filter_expr_ast* filterexprast = (struct filter_expr_ast*)ast;
            print_node(stream, level, "filter_expr: {\n");
            print_node(stream, level+1, "cmp: %s\n", str_cond[filterexprast->cmp]);
            print_ast(stream, filterexprast->attr_name, level+1);
            print_ast(stream, filterexprast->constant, level+1);
            print_node(stream, level, "}\n");
            break;
        }
        case NT_FILTER: {
            struct filter_ast* filterast = (struct filter_ast*)ast;
            print_node(stream, level, "filter: {\n");
            print_ast(stream, filterast->conditions_tree_root, level+1);
            print_node(stream, level, "}\n");
            break;
        }
        case NT_FOR: {
            struct for_ast* forast = (struct for_ast*)ast;
            print_node(stream, level, "for: {\n");
            print_node(stream, level+1, "var: %s\n", forast->var);
            print_node(stream, level+1, "tabname: %s\n", forast->tabname);
            if(forast->nonterm_list_head != NULL){
                print_node(stream, level+1, "body: [\n");
                print_ast(stream, forast->nonterm_list_head, level+1);
                print_node(stream, level+1, "]\n");
            }
            if(forast->terminal != NULL)
                print_ast(stream, forast->terminal, level+1);
            print_node(stream, level, "}\n");
            break;
        }
        case NT_INTVAL: {
            struct nint* intast = (struct nint*)ast;
            print_node(stream, level, "int: %d\n", intast->value);
            break;
        }
        case NT_FLOATVAL: {
            struct nfloat* floatast = (struct nfloat*)ast;
            print_node(stream, level, "float: %4.4f\n", floatast->value);
            break;
        }
        case NT_STRINGVAL: {
            struct nstring* stringast = (struct nstring*)ast;
            print_node(stream, level, "string: %s\n", stringast->value);
            break;
        }
        case NT_BOOLVAL: {
            struct nint* boolast = (struct nint*)ast;
            print_node(stream, level, "bool: %s\n", boolast->value ? "true" : "false");
            break;
        }

        default: {
            fprintf(stream, "unknown nodetype");
            return;
        }
    }
}

void free_ast(struct ast* ast){
    if(ast == NULL){
        return;
    }
    switch (ast->nodetype) {
        case NT_RETURN: {
            struct return_ast* returnast = (struct return_ast*)ast;
            free_ast(returnast->value);
            free(returnast);
            break;
        }
        case NT_INSERT: {
            struct insert_ast* insertast = (struct insert_ast*)ast;
            free(insertast->tabname);
            free_ast(insertast->list);
            free(insertast);
            break;
        }
        case NT_UPDATE: {
            struct update_ast* updateast = (struct update_ast*)ast;
            free(updateast->tabname);
            free_ast(updateast->attr);
            free_ast(updateast->list);
            free(updateast);
            break;
        }
        case NT_REMOVE: {
            struct remove_ast* removeast = (struct remove_ast*)ast;
            free(removeast->tabname);
            free_ast(removeast->attr);
            free(removeast);
            break;
        }
        case NT_CREATE: {
            struct create_ast* createast = (struct create_ast*)ast;
            free(createast->name);
            free_ast(createast->difinitions);
            free(createast);
            break;
        }
        case NT_DROP: {
            struct drop_ast* dropast = (struct drop_ast*)ast;
            free(dropast->name);
            free(dropast);
            break;
        }
        case NT_LIST: {
            struct list_ast* listast = (struct list_ast*)ast;
            free_ast(listast->value);
            free_ast(listast->next);
            free(listast);
            break;
        }
        case NT_PAIR : {
            struct pair_ast* pairast = (struct pair_ast*)ast;
            free_ast(pairast->value);
            free(pairast);
            break;
        }
        case NT_CREATE_PAIR: {
            struct create_pair_ast* create_pairast = (struct create_pair_ast*)ast;
            free(create_pairast->name);
            free(create_pairast);
            break;
        }
        case NT_MERGE: {
            struct merge_ast* mergeast = (struct merge_ast*)ast;
            free(mergeast->var1);
            free(mergeast->var2);
            free(mergeast);
            break;
        }
        case NT_ATTR_NAME: {
            struct attr_name_ast* attrnameast = (struct attr_name_ast*)ast;
            free(attrnameast->variable);
            if(attrnameast->attr_name != NULL)
                free(attrnameast->attr_name);
            free(attrnameast);
            break;
        }
        case NT_FILTER_CONDITION: {
            struct filter_condition_ast* cond_ast = (struct filter_condition_ast*)ast;
            free_ast(cond_ast->l);
            free_ast(cond_ast->r);
            free(cond_ast);
            break;
        }
        case NT_FILTER_EXPR: {
            struct filter_expr_ast* filterexprast = (struct filter_expr_ast*)ast;
            free_ast(filterexprast->attr_name);
            free_ast(filterexprast->constant);
            free(filterexprast);
            break;
        }
        case NT_FILTER: {
            struct filter_ast* filterast = (struct filter_ast*)ast;
            free_ast(filterast->conditions_tree_root);
            free(filterast);
            break;
        }
        case NT_FOR: {
            struct for_ast* forast = (struct for_ast*)ast;
            free(forast->var);
            free(forast->tabname);
            free_ast(forast->nonterm_list_head);
            free_ast(forast->terminal);
            free(forast);
            break;
        }
        case NT_INTVAL: {
            struct nint* intast = (struct nint*)ast;
            free(intast);
            break;
        }
        case NT_FLOATVAL: {
            struct nfloat* floatast = (struct nfloat*)ast;
            free(floatast);
            break;
        }
        case NT_STRINGVAL: {
            struct nstring* stringast = (struct nstring*)ast;
            free(stringast->value);
            free(stringast);
            break;
        }
        case NT_BOOLVAL: {
            struct nint* boolast = (struct nint*)ast;
            free(boolast);
            break;
        }

        default: {
            fprintf(stderr, "unknown nodetype");
            return;
        }
    }

}

