#ifndef PY3DENGINE_LOGGER_H
#define PY3DENGINE_LOGGER_H

extern void initLogger();
extern void debug_log(const char *format, ...);
extern void trace_log(const char *format, ...);
extern void info_log(const char *format, ...);
extern void warning_log(const char *format, ...);
extern void error_log(const char *format, ...);
extern void critical_log(const char *format, ...);

#endif
