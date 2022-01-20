#ifndef _CLOGGER_H
#define _CLOGGER_H

#include <stdio.h>

#define MSGMAXLENGTH 512

typedef enum Log_Type {
        Log_INFO,
        Log_DEBUG,
        Log_WARN,
        Log_ERROR
} Log_Type;

#define LOG_INFO(FORMAT, ...) clogger(Log_INFO, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)
#define LOG_DEBUG(FORMAT, ...) clogger(Log_DEBUG, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)
#define LOG_WARNING(FORMAT, ...) clogger(Log_WARN, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)
#define LOG_ERROR(FORMAT, ...) clogger(Log_ERROR, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

#define LOG_INFO_F(FILEPATH, FORMAT, ...) clogger_f(Log_INFO, FILEPATH, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)
#define LOG_DEBUG_F(FILEPATH, FORMAT, ...) clogger_f(Log_DEBUG, FILEPATH, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)
#define LOG_WARNING_F(FILEPATH, FORMAT, ...) clogger_f(Log_WARN, FILEPATH, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)
#define LOG_ERROR_F(FILEPATH, FORMAT, ...) clogger_f(Log_ERROR, FILEPATH, __FILE__, __func__, __LINE__, FORMAT, ##__VA_ARGS__)

void clogger(Log_Type type, const char *filename, const char *function, int line, const char *format, ...);
void clogger_f(Log_Type type, const char *filepath, const char *filename, const char *function, int line, const char *format, ...);

#endif
