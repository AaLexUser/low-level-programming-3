#ifndef AST_H
#define AST_H

/* interface to the lexer */
extern int yylineno; /* from lexer */
#include <stdio.h>


typedef enum ntype {
    /* keywords */
    NT_FOR, NT_RETURN, NT_FILTER, NT_INSERT,
    NT_UPDATE, NT_REMOVE, NT_CREATE, NT_DROP,
    NT_PAIR, NT_FILTER_CONDITION, NT_FILTER_EXPR,
    NT_ATTR_NAME, NT_LIST, NT_CREATE_PAIR, NT_MERGE, NT_MERGE_PROJECTIONS,

    /* variable types */
    NT_INTEGER, NT_FLOAT, NT_STRING, NT_BOOLEAN,

    /* comparison ops */
    NT_EQ, NT_NEQ, NT_LT, NT_LTE, NT_GT, NT_GTE,
    NT_AND, NT_OR, NT_IN,

    /* values */
    NT_INTVAL, NT_FLOATVAL, NT_STRINGVAL, NT_BOOLVAL
} ntype_t;

static char* str_cond[] ={
    [NT_EQ] = "EQ",
    [NT_NEQ] = "NEQ",
    [NT_LT] = "LT",
    [NT_LTE] = "LTE",
    [NT_GT] = "GT",
    [NT_GTE] = "GTE",
    [NT_AND] = "AND",
    [NT_OR] = "OR",
    [NT_IN] = "IN",
};

static char* str_type[] ={
    [NT_INTEGER] = "int",
    [NT_FLOAT] = "float",
    [NT_STRING] = "string",
    [NT_BOOLEAN] = "bool",
};


struct ast {
    ntype_t nodetype;
    struct ast *l;
    struct ast *r;
};

struct nint {
    ntype_t nodetype;
    int value;
};

struct nfloat {
    ntype_t nodetype;
    double value;
};

struct nstring {
    ntype_t nodetype;
    char* value;
};

struct for_ast {
    ntype_t nodetype;
    char* var; 
    char* tabname;
    struct ast* nonterm_list_head;
    struct ast* terminal;
};

struct list_ast {
    ntype_t nodetype;
    struct ast* value;
    struct ast* next;
};

struct filter_ast {
    ntype_t nodetype;
    struct ast* conditions_tree_root;
};

struct filter_condition_ast {
    ntype_t nodetype;
    struct ast* l;
    struct ast* r;
    int logic;
};

struct filter_expr_ast {
    ntype_t nodetype;
    struct ast* attr_name;
    struct ast* constant;
    int cmp;
};

struct attr_name_ast {
    ntype_t nodetype;
    char* variable;
    char* attr_name;
};

struct return_ast {
    ntype_t nodetype;
    struct ast* value;
};

struct merge_ast {
    ntype_t nodetype;
    char* var1;
    char* var2;
};

struct merge_projections_ast {
    ntype_t nodetype;
    struct ast* list;
};

struct pair_ast {
    ntype_t nodetype;
    char* key;
    struct ast* value;
};

struct condition_ast {
    ntype_t nodetype;
    struct ast* l;
    struct ast* r;
};

struct variable_ast {
    ntype_t nodetype;
    char* name;
};

struct insert_ast {
    ntype_t nodetype;
    char* tabname;
    struct ast* list;
};

struct update_ast {
    ntype_t nodetype;
    char* tabname;
    struct ast* attr;
    struct ast* list;
};

struct remove_ast {
    ntype_t nodetype;
    char* tabname;
    struct ast* attr;
};

struct create_pair_ast {
    ntype_t nodetype;
    char* name;
    ntype_t type;
};

struct create_ast {
    ntype_t nodetype;
    char* name;
    struct ast* difinitions;
};

struct drop_ast {
    ntype_t nodetype;
    char* name;
};



struct ast*
newast(ntype_t nodetype, struct ast* l, struct ast* r);

struct ast*
newint(int value);

struct ast*
newfloat(double value);

struct ast*
newstring(char* value);

struct ast*
newbool(int value);

struct ast*
newfor(char* var, char* tabname, struct ast* nonterm_list_head, struct ast* terminal);

struct ast*
newlist(struct ast* value, struct ast* next);

struct ast*
newfilter(struct ast* conditions_tree_root);

struct ast*
newfilter_condition(struct ast* l, struct ast* r, int logic);

struct ast*
newfilter_expr(struct ast* attr_name, struct ast* constant, int cmp);

struct ast*
newattr_name(char* variable, char* attr_name);

struct ast*
newpair(char* key, struct ast* value);

struct ast*
newreturn(struct ast* value);

struct ast*
newmerge(char* var1, char* var2);

struct ast*
newmerge_projections(struct ast* list);

struct ast*
newinsert(char* tabname, struct ast* list);

struct ast*
newupdate(char* tabname, struct ast* attr, struct ast* list);

struct ast*
newremove(char* tabname, struct ast* attr);

struct ast*
newcreate_pair(char* name, int type);

struct ast*
newcreate(char* name, struct ast* difinitions);

struct ast*
newdrop(char* name);



void print_ast(FILE* stream, struct ast* ast, int level);
void free_ast(struct ast* ast);








#endif