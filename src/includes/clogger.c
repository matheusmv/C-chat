#include "clogger.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ACTIVE   true
#define INACTIVE false

typedef void (*logger_fn)(FILE *stream, const char *time, Log_Type type, const char *filename, const char *function, int line, const char *format, va_list args);

typedef struct Clogger Clogger;
static struct Clogger {
        FILE *console_stream;
        bool colored;
        bool initialized;
        logger_fn file_logger;
        logger_fn console_logger;
} _logger;

typedef void (*init_fn)(pthread_mutex_t *mutex);
typedef void (*lock_fn)(pthread_mutex_t *mutex);
typedef void (*unlock_fn)(pthread_mutex_t *mutex);

typedef struct CloggerMutex CloggerMutex;
static struct CloggerMutex {
        pthread_mutex_t mutex;
        init_fn init;
        lock_fn lock;
        unlock_fn unlock;
} _mutex;

static void
init(pthread_mutex_t *mutex)
{
        pthread_mutex_init(mutex, NULL);
}

static void
lock(pthread_mutex_t *mutex)
{
        pthread_mutex_lock(mutex);
}

static void
unlock(pthread_mutex_t *mutex)
{
        pthread_mutex_unlock(mutex);
}

static struct CloggerMutex
CloggerMutex_create(void)
{
        return (struct CloggerMutex) { .init = init, .lock = lock, .unlock = unlock };
}

static void
clogger_log(FILE *stream, const char *time, const char *log_level, const char *filename, const char *function, int line, const char *format, va_list args)
{
        char message[MSGMAXLENGTH];
        vsnprintf(message, sizeof message, format, args);
        fprintf(stream, "%s %s %s:'%s':%d - %s\n", time, log_level, filename, function, line, message);
}

static char *
clogger_type_string(Log_Type type, bool colored)
{
        switch (type) {
        case Log_INFO:
                return colored ? "\x1B[32mINFO\033[0m " : "INFO ";
        case Log_DEBUG:
                return colored ? "\x1B[34mDEBUG\033[0m" : "DEBUG";
        case Log_WARN:
                return colored ? "\x1B[33mWARN\033[0m " : "WARN ";
        case Log_ERROR:
                return colored ? "\x1B[31mERROR\033[0m" : "ERROR";
        default:
                return "UNDEF";
        }
}

static void
log_to_file(FILE *stream, const char *time, Log_Type type, const char *filename, const char *function, int line, const char *format, va_list args)
{
        clogger_log(stream, time, clogger_type_string(type, INACTIVE), filename, function, line, format, args);
}

static void
log_to_console(FILE *stream, const char *time, Log_Type type, const char *filename, const char *function, int line, const char *format, va_list args)
{
        clogger_log(stream, time, clogger_type_string(type, _logger.colored), filename, function, line, format, args);
}

static void
clogger_init(void)
{
        _mutex = CloggerMutex_create();
        _mutex.init(&_mutex.mutex);

        _mutex.lock(&_mutex.mutex);

        _logger.console_stream = stderr;
#ifdef LCOLOR
        _logger.colored = ACTIVE;
#else
        _logger.colored = INACTIVE;
#endif
        _logger.initialized = true;
        _logger.file_logger = log_to_file;
        _logger.console_logger = log_to_console;

        _mutex.unlock(&_mutex.mutex);
}

static void
get_time(char *dest, size_t dest_size)
{
        assert(dest_size >= 22);

        time_t seconds;
        struct tm time_info;

        seconds = time(NULL);
        localtime_r(&seconds, &time_info);

        strftime(dest, dest_size, "%b %d %Y %X", &time_info);
}

void
clogger(Log_Type type, const char *filename, const char *function, int line, const char *format, ...)
{
        if (!_logger.initialized) {
                clogger_init();
        }

        va_list args;

        char date_time[32];

        _mutex.lock(&_mutex.mutex);

        va_start(args, format);

        get_time(date_time, sizeof date_time);

        _logger.console_logger(_logger.console_stream, date_time, type, filename, function, line, format, args);

        va_end(args);

        _mutex.unlock(&_mutex.mutex);
}

void
clogger_f(Log_Type type, const char *filepath, const char *filename, const char *function, int line, const char *format, ...)
{
        if (!_logger.initialized) {
                clogger_init();
        }

        va_list args;

        char date_time[32];

        _mutex.lock(&_mutex.mutex);

        FILE *file = NULL;
        if ((file = fopen(filepath, "a")) == NULL) {
                fprintf(stderr, "%s - %s\n", strerror(errno), filepath);
                _mutex.unlock(&_mutex.mutex);
                return;
        }

        va_start(args, format);

        get_time(date_time, sizeof date_time);

        _logger.file_logger(file, date_time, type, filename, function, line, format, args);

        va_end(args);

        fclose(file);

        _mutex.unlock(&_mutex.mutex);
}
