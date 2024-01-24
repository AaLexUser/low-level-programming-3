#ifndef ROW_LIKEDLIST_H
#define ROW_LIKEDLIST_H
#include <inttypes.h>
#include "backend/table/schema.h"
#include "backend/table/table_base.h"

typedef struct rst_node{
    chblix_t rowix;
    schema_t *schema;
    table_t* table;
    struct rst_node *next;
} rst_node_t;

typedef struct row_node {
    rst_node_t* rst_head;
    rst_node_t* rst_tail;
    int64_t rst_size;
    struct row_node *next;
    struct row_node *prev;
    uint8_t* row;
} row_node_t;


typedef struct row_likedlist {
    row_node_t *head;
    row_node_t *tail;
    int size;
    schema_t *schema;
} row_likedlist_t;

typedef struct row_likedlist_iterator {
    row_node_t *current;
} row_likedlist_iterator_t;

row_likedlist_t *row_likedlist_init(schema_t *schema);
void row_likedlist_free(row_likedlist_t *list);
int row_likedlist_add(row_likedlist_t *list,
                      chblix_t* rowix,
                      void* row,
                      schema_t* origin_schema,
                      table_t* origin_table );
void row_likedlist_add_rst(chblix_t* rowix, row_node_t* row_node, schema_t* schema, table_t* table);
void row_likedlist_remove(row_likedlist_t *list, row_node_t *node);



#endif