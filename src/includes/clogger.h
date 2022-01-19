#ifndef _CLOGGER_H
#define _CLOGGER_H

#include <stdbool.h>
#include <stdio.h>

#define MSGMAXLENGTH 512

#define ACTIVE   true
#define INACTIVE false

typedef enum LogType {
        LOG_INFO,
        LOG_DEBUG,
        LOG_WARN,
        LOG_ERROR
} LogType;

void clogger_colored_console(bool status);
void clogger(LogType type, const char *filename, const char *function, int line, const char *format, ...);
void clogger_f(LogType type, FILE *stream, const char *filename, const char *function, int line, const char *format, ...);

#define LOG_INFO(FORMAT, ...) clogger(LOG_INFO, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_INFO_F(STREAM, FORMAT, ...) clogger_f(LOG_INFO, STREAM, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_DEBUG(FORMAT, ...) clogger(LOG_DEBUG, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_DEBUG_F(STREAM, FORMAT, ...) clogger_f(LOG_DEBUG, STREAM, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_WARNING(FORMAT, ...) clogger(LOG_WARN, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_WARNING_F(STREAM, FORMAT, ...) clogger_f(LOG_WARN, STREAM, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_ERROR(FORMAT, ...) clogger(LOG_ERROR, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_ERROR_F(STREAM, FORMAT, ...) clogger_f(LOG_ERROR, STREAM, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#endif
