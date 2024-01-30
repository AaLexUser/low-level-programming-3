#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief       This function creates a formatted string.
 * @param[in]   format: A format string similar to printf.
 * @param[in]   ...: Additional arguments to fill the format string.
 * @return      A pointer to the newly created string.
 */
char *strdupf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int length = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *str = malloc(length + 1);
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);
    return str;
}