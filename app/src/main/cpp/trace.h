#pragma once

#include <stdio.h>
#include <stdarg.h>

void print_line(FILE * fs, int sec, const char * fmt, ...);
void print_short(FILE * fs, const char * fmt, ...);

