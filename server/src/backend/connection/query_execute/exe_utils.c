#include <stdarg.h>
#include <stdio.h>
#include "exe_utils.h"

/**
 * @brief       This function logs an error message and updates the response structure.
 * @param[in]   resp: A pointer to the response structure to be updated.
 * @param[in]   message: A format string for the error message.
 * @param[in]   ...: Additional arguments to fill the format string.
 * @return      Always returns -1 as an indication of error.
 */
int log_error_and_update_response(struct response *resp, const char *message, ...) {
    va_list args;
    va_start(args, message);
    int length = vsnprintf(NULL, 0, message, args);
    va_end(args);
    char *str = malloc(length + 1);
    va_start(args, message);
    vsprintf(str, message, args);
    va_end(args);
    logger(LL_ERROR, __func__, str);
    resp->status = -1;
    resp->message = str;
    return -1;
}

/**
 * @brief       This function reverses the order of a linked list.
 * @param[in]   head_ref: A pointer to the head of the list.
 */
void reverseList(struct list_ast **head_ref) {
    struct list_ast *prev = NULL;
    struct list_ast *current = *head_ref;
    struct list_ast *next = NULL;

    while (current != NULL) {
        next = (struct list_ast *) current->next;
        current->next = (struct ast *) prev;
        prev = current;
        current = next;
    }

    *head_ref = prev;
}

condition_t get_condition_type(int comparison_type) {
    if (comparison_type < 0 || comparison_type >= sizeof(condition_lookup) / sizeof(condition_lookup[0])) {
        logger(LL_ERROR, __func__, "Invalid condition %d", comparison_type);
        return -1;
    }
    return condition_lookup[comparison_type];
}

struct constant_val *get_constant(struct ast *constant) {
    if (constant->nodetype < 0 || constant->nodetype >= sizeof(datatype_lookup) / sizeof(datatype_lookup[0])) {
        logger(LL_ERROR, __func__, "Invalid constant type %d", constant->nodetype);
        return NULL;
    }

    struct constant_val *constant_val = malloc(sizeof(struct constant_val));
    constant_val->type = datatype_lookup[constant->nodetype];

    switch (constant_val->type) {
        case DT_INT:
            constant_val->int_val = ((struct nint *) constant)->value;
            break;
        case DT_FLOAT:
            constant_val->float_val = ((struct nfloat *) constant)->value;
            break;
        case DT_BOOL:
            constant_val->bool_val = ((struct nint *) constant)->value;
            break;
        case DT_VARCHAR:
            constant_val->string_val = ((struct nstring *) constant)->value;
            break;
        default:
            logger(LL_ERROR, __func__, "Invalid constant type %d", constant->nodetype);
            free(constant_val);
            return NULL;
    }

    return constant_val;
}