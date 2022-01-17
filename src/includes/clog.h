#ifndef _CLOG_H
#define _CLOG_H

#include <stdio.h>
#include <time.h>

#define ULINE "\x1b[4m"

#define CRED "\x1B[31m"
#define CGRN "\x1B[32m"
#define CYEL "\x1B[33m"
#define CBLU "\x1B[34m"
#define CWHT "\x1b[97m"
#define CEND "\033[0m"

/* DATE & TIME - Weekday Month Year hh:mm:ss */
static inline void
get_time(char *src, size_t size)
{
        time_t t;
        struct tm time_info;

        tzset();
        t = time(NULL);
        localtime_r(&t, &time_info);

        strftime(src, size, "%b %d %Y %X", &time_info);
}

/* INFO */

#define LOG_INFO(MSG, ...) \
{\
        char dt[32];\
        get_time(dt, sizeof(dt));\
        fprintf(stderr, "%s %sINFO%2s%s %s%s:'%s':%d%s - %s" MSG "%s\n", \
                dt, CGRN, "", CEND, ULINE, __FILE__, __func__, __LINE__, CEND, CWHT, ##__VA_ARGS__, CEND);\
}

#define FLOG_INFO(STREAM, MSG, ...) \
{\
        char dt[32];\
        get_time(dt, sizeof(dt));\
        fprintf(STREAM, "%s INFO%2s %s:'%s':%d - " MSG "\n", \
                dt, "", __FILE__, __func__, __LINE__, ##__VA_ARGS__);\
}

/* DEBUG */

#define LOG_DEBUG(MSG, ...) \
{\
        char dt[32];\
        get_time(dt, sizeof(dt));\
        fprintf(stderr, "%s %sDEBUG%1s%s %s%s:'%s':%d%s - %s" MSG "%s\n", \
                dt, CBLU, "", CEND, ULINE, __FILE__, __func__, __LINE__, CEND, CWHT, ##__VA_ARGS__, CEND);\
}

#define FLOG_DEBUG(STREAM, MSG, ...) \
{\
       char dt[32];\
       get_time(dt, sizeof(dt));\
       fprintf(STREAM, "%s DEBUG%1s %s:'%s':%d - " MSG "\n", \
               dt, "", __FILE__, __func__, __LINE__, ##__VA_ARGS__);\
}

/* WARNING */

#define LOG_WARNING(MSG, ...) \
{\
        char dt[32];\
        get_time(dt, sizeof(dt));\
        fprintf(stderr, "%s %sWARN%2s%s %s%s:'%s':%d%s - %s" MSG "%s\n", \
                dt, CYEL, "", CEND, ULINE, __FILE__, __func__, __LINE__, CEND, CWHT, ##__VA_ARGS__, CEND);\
}

#define FLOG_WARNING(STREAM, MSG, ...) \
{\
        char dt[32];\
        get_time(dt, sizeof(dt));\
        fprintf(STREAM, "%s WARN%2s %s:'%s':%d - " MSG "\n", \
                dt, "", __FILE__, __func__, __LINE__, ##__VA_ARGS__);\
}

/* ERROR */

#define LOG_ERROR(MSG, ...) \
{\
        char dt[32];\
        get_time(dt, sizeof(dt));\
        fprintf(stderr, "%s %sERROR%1s%s %s%s:'%s':%d%s - %s" MSG "%s\n", \
                dt, CRED, "", CEND, ULINE, __FILE__, __func__, __LINE__, CEND, CWHT, ##__VA_ARGS__, CEND);\
}

#define FLOG_ERROR(STREAM, MSG, ...) \
{\
        char dt[32];\
        get_time(dt, sizeof(dt));\
        fprintf(STREAM, "%s ERROR%1s %s:'%s':%d - " MSG "\n", \
                dt, "", __FILE__, __func__, __LINE__, ##__VA_ARGS__);\
}

#endif
