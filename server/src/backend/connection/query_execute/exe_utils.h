#ifndef EXE_UTILS_H
#define EXE_UTILS_H
#include "backend/data_type.h"
#include "backend/table/table.h"
#include "backend/connection/ast.h"

struct constant_val {
    datatype_t type;
    union {
        int64_t int_val;
        double float_val;
        bool bool_val;
        char *string_val;
    };
};

struct response {
    int status;
    char* message;
    table_t* table;
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

int log_error_and_update_response(struct response *resp, const char *message, ...);
void reverseList(struct list_ast **head_ref);
condition_t get_condition_type(int comparison_type);
struct constant_val *get_constant(struct ast *constant);

#endif