#include "row_likedlist.h"
#include "backend/table/table_base.h"
#include <stdlib.h>


row_likedlist_t *row_likedlist_init(schema_t *schema)
{
    row_likedlist_t *list = malloc(sizeof(row_likedlist_t));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->schema = schema;
    return list;
}

row_node_t* init_row_node(){
    row_node_t* node = malloc(sizeof(row_node_t));
    if (node == NULL) {
        logger(LL_ERROR, __func__, "Failed to allocate memory for row_node_t");
        return NULL;
    }
    node->row = NULL;
    node->rst_head = NULL;
    node->rst_tail = NULL;
    node->next = NULL;
    node->prev = NULL;
    node->rst_size = 0;
    return node;
}

static void free_node(row_node_t* node){
    rst_node_t *current = node->rst_head;
    while (current != NULL)
    {
        rst_node_t *next = current->next;
        free(current);
        current = next;
    }

    free(node->row);
    free(node);
}

void row_likedlist_free(row_likedlist_t *list)
{
    row_node_t *current = list->head;
    while (current != NULL)
    {
        row_node_t *next = current->next;
        free_node(current);
        current = next;
    }
    free(list);
}

void row_likedlist_add_rst(chblix_t* rowix, row_node_t* row_node, schema_t* schema, table_t* table)
{
    rst_node_t *node = malloc(sizeof(rst_node_t));
    node->rowix = *rowix;
    node->schema = schema;
    node->table = table;
    node->next = NULL;
    if (row_node->rst_head == NULL){
        row_node->rst_head = node;
        row_node->rst_tail = node;
    }
    else{
        row_node->rst_tail->next = node;
        row_node->rst_tail = node;
    }
}

int row_likedlist_add(row_likedlist_t *list,
                       chblix_t* rowix,
                       void* row,
                       schema_t* origin_schema,
                       table_t* origin_table )
{
    if(origin_schema == NULL){
        logger(LL_ERROR, __func__, "Invalid argument: schema is NULL");
        return -1;
    }

    if(origin_table == NULL){
        logger(LL_ERROR, __func__, "Invalid argument: table is NULL");
        return -1;
    }

    if(rowix == NULL){
        logger(LL_ERROR, __func__, "Invalid argument: rowix is NULL");
        return -1;
    }


    row_node_t *node = init_row_node();
    row_likedlist_add_rst(rowix, node, origin_schema, origin_table);
    node->row = malloc(list->schema->slot_size);
    memcpy(node->row, row, list->schema->slot_size);
    node->next = NULL;
    node->prev = NULL;
    if (list->head == NULL)
    {
        list->head = node;
        list->tail = node;
    }
    else
    {
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
    }
    list->size++;
    return 0;
}



void row_likedlist_remove(row_likedlist_t *list, row_node_t *node)
{
    if (node->prev == NULL)
    {
        list->head = node->next;
    }
    else
    {
        node->prev->next = node->next;
    }
    if (node->next == NULL)
    {
        list->tail = node->prev;
    }
    else
    {
        node->next->prev = node->prev;
    }
    list->size--;
    free_node(node);
}

