#include <stdio.h>
#include <stdarg.h>

#include "logger.h"

#define LOG_DEBUG "DEBUG"
#define LOG_TRACE "TRACE"
#define LOG_INFO "INFO"
#define LOG_WARNING "WARNING"
#define LOG_ERROR "ERROR"
#define LOG_CRITICAL "CRITICAL"

static FILE *debugfd;
static FILE *tracefd;
static FILE *infofd;
static FILE *warningfd;
static FILE *errorfd;
static FILE *criticalfd;

static void level_log(FILE *fd, const char *level, const char *message, va_list args) {
    fprintf(fd, "[%s]: ", level);
    vfprintf(fd, message, args);
    fprintf(fd, "\n");
}

void initLogger() {
    debugfd = stdout;
    tracefd = stdout;
    infofd = stdout;
    warningfd = stderr;
    errorfd = stderr;
    criticalfd = stderr;
}

void debug_log(const char *format, ...) {
    va_list args;
    va_start(args, format);

    level_log(debugfd, LOG_DEBUG, format, args);

    va_end(args);
}

void trace_log(const char *format, ...) {
    va_list args;
    va_start(args, format);

    level_log(tracefd, LOG_TRACE, format, args);

    va_end(args);
}

void info_log(const char *format, ...) {
    va_list args;
    va_start(args, format);

    level_log(infofd, LOG_INFO, format, args);

    va_end(args);
}

void warning_log(const char *format, ...) {
    va_list args;
    va_start(args, format);

    level_log(warningfd, LOG_WARNING, format, args);

    va_end(args);
}

void error_log(const char *format, ...) {
    va_list args;
    va_start(args, format);

    level_log(errorfd, LOG_ERROR, format, args);

    va_end(args);
}

void critical_log(const char *format, ...) {
    va_list args;
    va_start(args, format);

    level_log(criticalfd, LOG_CRITICAL, format, args);

    va_end(args);
}