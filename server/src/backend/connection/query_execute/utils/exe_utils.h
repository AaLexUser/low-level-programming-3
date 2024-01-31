#ifndef EXE_UTILS_H
#define EXE_UTILS_H

#include "backend/data_type.h"
#include "backend/table/table.h"
#include "backend/connection/ast.h"


struct response {
    int status;
    char *message;
    table_t *table;
};

static const condition_t condition_lookup[] = {
        [NT_EQ] = COND_EQ,
        [NT_NEQ] = COND_NEQ,
        [NT_LT] = COND_LT,
        [NT_LTE] = COND_LTE,
        [NT_GT] = COND_GT,
        [NT_GTE] = COND_GTE
};

static const datatype_t datatype_lookup[] = {
        [NT_INTVAL] = DT_INT,
        [NT_FLOATVAL] = DT_FLOAT,
        [NT_BOOLVAL] = DT_BOOL,
        [NT_STRINGVAL] = DT_VARCHAR
};
/**
 * @brief       This function logs an error message and updates the response structure.
 * @param[in]   resp: A pointer to the response structure to be updated.
 * @param[in]   msg: A format string for the error message.
 * @param[in]   ...: Additional arguments to fill the format string.
 * @return      Always returns -1 as an indication of error.
 */

#define LOG_ERROR_AND_UPDATE_RESPONSE(resp, msg, ...) do { \
    char* str;                                                      \
    if (#__VA_ARGS__[0] == '\0') { \
        str = strdup(msg); \
    } else {                                               \
        str = format_string(msg, ##__VA_ARGS__);     \
    }\
    logger(LL_ERROR, __func__, str); \
    resp->status = -1; \
    resp->message = str; \
} while(0)

struct response* create_response();

void reverseList(struct list_ast **head_ref);

condition_t get_condition_type(int comparison_type);

int set_response(struct response *resp, int status, const char *message, ...);

char *format_string(const char *message, ...) __attribute__((format(printf, 1, 2)));

#endif