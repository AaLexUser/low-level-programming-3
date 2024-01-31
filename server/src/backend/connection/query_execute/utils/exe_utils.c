#include <stdarg.h>
#include <stdio.h>
#include "exe_utils.h"
#include "utils/utils.h"

struct response* create_response() {
    struct response* resp = malloc(sizeof(struct response));
    if (resp == NULL) {
        logger(LL_ERROR, __func__, "Failed to allocate memory for response");
        return NULL;
    }
    resp->status = 0;
    resp->message = NULL;
    resp->table = NULL;
    return resp;
}

char* format_string(const char *message, ...)  {
    va_list args;
    va_start(args, message);
    va_list args_copy;
    va_copy(args_copy, args);
    int length = vsnprintf(NULL, 0, message, args);
    char *str = malloc(length + 1);
    if (str == NULL) {
        logger(LL_ERROR, __func__, "Failed to allocate memory for string");
        return NULL;
    }
    vsnprintf(str, length + 1, message, args_copy);
    va_end(args_copy);
    return str;
}


int set_response(struct response *resp, int status, const char *message, ...) {
    va_list args;
    va_start(args, message);
    char *str = format_string(message, args);
    va_end(args);
    resp->status = status;
    resp->message = str;
    return status;
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

