#include "clogger.h"

#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct ClogConsole ClogConsole;
struct ClogConsole {
        FILE *stream;
        bool colored;
        bool initialized;
};

typedef void (*init_fn)(pthread_mutex_t *mutex);
typedef void (*lock_fn)(pthread_mutex_t *mutex);
typedef void (*unlock_fn)(pthread_mutex_t *mutex);

typedef struct ClogMutex ClogMutex;
struct ClogMutex {
        pthread_mutex_t mutex;
        init_fn init;
        lock_fn lock;
        unlock_fn unlock;
};

static struct ClogConsole _console_logger;
static struct ClogMutex _mutex;

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

static struct ClogMutex
ClogMutex_create(void)
{
        return (struct ClogMutex) { .init = init, .lock = lock, .unlock = unlock };
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

static char *
clogger_Level_string(Level level)
{
        switch (level) {
        case LOG_INFO:
                return _console_logger.colored ? "\x1B[32mINFO\033[0m " : "INFO ";
        case LOG_DEBUG:
                return _console_logger.colored ? "\x1B[34mDEBUG\033[0m" : "DEBUG";
        case LOG_WARN:
                return _console_logger.colored ? "\x1B[33mWARN\033[0m " : "WARN ";
        case LOG_ERROR:
                return _console_logger.colored ? "\x1B[31mERROR\033[0m" : "ERROR";
        default:
                return "UNDEF";
        }
}

static void
clogger_log(FILE *stream, const char *time, const char *log_level, const char *filename, const char *function, int line, const char *format, va_list args)
{
        char message[1024];
        vsnprintf(message, sizeof message, format, args);
        fprintf(stream, "%s %s %s:'%s':%d - %s\n", time, log_level, filename, function, line, message);
}

void
clogger_create(FILE *stream)
{
        stream = (stream == NULL) ? stderr : stream;

        _mutex = ClogMutex_create();
        _mutex.init(&_mutex.mutex);

        _mutex.lock(&_mutex.mutex);

        _console_logger.stream = stream;
        _console_logger.colored = (stream == stderr) || (stream == stdout) ? true : false;
        _console_logger.initialized = true;

        _mutex.unlock(&_mutex.mutex);
}

void
clogger(Level level, const char *filename, const char *function, int line, const char *format, ...)
{
        assert(_console_logger.initialized == true);

        char date_time[32];

        va_list args;

        get_time(date_time, sizeof date_time);

        _mutex.lock(&_mutex.mutex);

        va_start(args, format);

        clogger_log(_console_logger.stream, date_time, clogger_Level_string(level), filename, function, line, format, args);

        va_end(args);

        _mutex.unlock(&_mutex.mutex);
}
