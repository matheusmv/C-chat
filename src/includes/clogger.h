#ifndef _CLOGGER_H
#define _CLOGGER_H

#include <stdio.h>

typedef enum Level {
        LOG_INFO,
        LOG_DEBUG,
        LOG_WARN,
        LOG_ERROR
} Level;

void clogger_create(FILE *stream);

void clogger(Level level, const char *filename, const char *function, int line, const char *format, ...);

#define LOG_INFO(FORMAT, ...) clogger(LOG_INFO, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_DEBUG(FORMAT, ...) clogger(LOG_DEBUG, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_WARNING(FORMAT, ...) clogger(LOG_WARN, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_ERROR(FORMAT, ...) clogger(LOG_ERROR, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#endif