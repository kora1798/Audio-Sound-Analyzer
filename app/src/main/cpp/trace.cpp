#include "trace.h"
#include <string.h>

void print___(FILE * fs, const char * str)
{
	printf("%s", str);
	if(fs){
		fprintf(fs, "%s", str);
		fflush(fs);
	}
}
	
static bool b_line_prev;
void print_line(FILE * fs, int sec, const char * fmt, ...)
{
	#define LEN 1000
    char str[LEN];
	sprintf(str, "%02i:%02i:%02i ", sec/3600, sec/60, sec%60);
	int len = strlen(str);
	
	va_list ap;
	va_start(ap, fmt);
	//_vsnprintf_s(str, sizeof(str), fmt, ap);
	vsnprintf(str+len, LEN-len, fmt, ap);
	va_end(ap);
	if(!b_line_prev)
		print___(fs, "\n");
	print___(fs, str);
	print___(fs, "\n");
	b_line_prev = true;
}

void print_short(FILE * fs, const char * fmt, ...)
{
    char str[1000];
	va_list ap;
	va_start(ap, fmt);
	//_vsnprintf_s(str, sizeof(str), fmt, ap);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);
	print___(fs, str);
	b_line_prev = false;

}
