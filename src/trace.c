#include "trace.h"
#include <stdarg.h>

FILE *trace_file = NULL;

void trace(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    if (trace_file) {
        va_start(args, format);
        vfprintf(trace_file, format, args);
        va_end(args);
        fflush(trace_file);
    }
}

// int sntrace(char *buf, size_t size, const char *format, ...)
// {
//     va_list args;
//     int n;

//     va_start(args, format);
//     n = vsnprintf(buf, size, format, args);
//     va_end(args);

//     printf("%s", buf);

//     if (trace_file && n >= 0) {
//         fprintf(trace_file, "%s", buf);
//         fflush(trace_file);
//     }

//     return n;
// }
