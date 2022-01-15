#ifndef _CLOG_H
#define _CLOG_H

#include <stdio.h>

#define ULINE "\x1b[4m"

#define CRED "\x1B[31m"
#define CGRN "\x1B[32m"
#define CYEL "\x1B[33m"
#define CBLU "\x1B[34m"
#define CWHT "\x1b[97m"
#define CEND "\033[0m"

/* INFO */

#define LOG_INFO(MSG, ...) \
        fprintf(stdout, "%s %s %sINFO%2s%s %s%s:'%s':%d%s - %s" MSG "%s\n",\
                __DATE__, __TIME__, CGRN, "", CEND, ULINE, __FILE__, __func__, __LINE__, CEND, CWHT, ##__VA_ARGS__, CEND)

#define FLOG_INFO(STREAM, MSG, ...) \
        fprintf(STREAM, "%s %s INFO%2s %s:'%s':%d - " MSG "\n",\
                __DATE__, __TIME__, "", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

/* DEBUG */

#define LOG_DEBUG(MSG, ...) \
        fprintf(stdout, "%s %s %sDEBUG%1s%s %s%s:'%s':%d%s - %s" MSG "%s\n",\
                __DATE__, __TIME__, CBLU, "", CEND, ULINE, __FILE__, __func__, __LINE__, CEND, CWHT, ##__VA_ARGS__, CEND)

#define FLOG_DEBUG(STREAM, MSG, ...) \
        fprintf(STREAM, "%s %s DEBUG%1s %s:'%s':%d - " MSG "\n",\
                __DATE__, __TIME__, "", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

/* WARNING */

#define LOG_WARNING(MSG, ...) \
        fprintf(stdout, "%s %s %sWARN%2s%s %s%s:'%s':%d%s - %s" MSG "%s\n",\
                __DATE__, __TIME__, CYEL, "", CEND, ULINE, __FILE__, __func__, __LINE__, CEND, CWHT, ##__VA_ARGS__, CEND)

#define FLOG_WARNING(STREAM, MSG, ...) \
        fprintf(STREAM, "%s %s WARN%2s %s:'%s':%d - " MSG "\n",\
                __DATE__, __TIME__, "", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

/* ERROR */

#define LOG_ERROR(MSG, ...) \
        fprintf(stderr, "%s %s %sERROR%1s%s %s%s:'%s':%d%s - %s" MSG "%s\n",\
                __DATE__, __TIME__, CRED, "", CEND, ULINE, __FILE__, __func__, __LINE__, CEND, CWHT, ##__VA_ARGS__, CEND)

#define FLOG_ERROR(STREAM, MSG, ...) \
        fprintf(STREAM, "%s %s ERROR%1s %s:'%s':%d - " MSG "\n",\
                __DATE__, __TIME__, "", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#endif
