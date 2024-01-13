#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static char* get_current_time_str(void){
    time_t now;
    time(&now);
    char *time_str = ctime(&now);
    return time_str;
}

static char* get_log_level_name(enum LoggerLevel ll){
    switch (ll) {
        case LL_DEBUG:
            return "DEBUG";
        case LL_INFO:
            return "INFO";
        case LL_WARN:
            return "WARN";
        case LL_ERROR:
            return "ERROR";
        default:
            return "UNDEFINED_LOG_LEVEL";
    }
}

void logger(enum LoggerLevel ll, const char *tag, const char *message, ...){
    if(LOGGER_LEVEL <= ll){
        va_list args;
        va_start(args, message);

        if (ll == LL_ERROR) {
            printf("\033[0;31m");
        } else if (ll == LL_WARN) {
            printf("\033[0;33m");
        } else if (ll == LL_INFO) {
            printf("\033[0;34m");
        }

        char* time = get_current_time_str();

        fprintf(stdout, "%s - %s [%s]: ", time, get_log_level_name(ll), tag);
        vprintf(message, args);
        va_end(args);
        printf("\n");
        if(ll == LL_ERROR || ll == LL_WARN || ll == LL_INFO){
            printf("\033[0m");
        }
    }
}