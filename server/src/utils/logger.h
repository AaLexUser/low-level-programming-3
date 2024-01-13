#pragma once

#ifndef LOGGER_LEVEL
#define LOGGER_LEVEL LL_DEBUG
#endif

enum LoggerLevel {
    LL_DEBUG = 0,
    LL_INFO = 1,
    LL_WARN = 2,
    LL_ERROR = 3
};

void logger(enum LoggerLevel ll, const char *tag, const char *message, ...);