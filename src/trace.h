#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>

extern FILE *trace_file;

void trace(const char *format, ...);
int sntrace(char *buf, size_t size, const char *format, ...);

#endif
