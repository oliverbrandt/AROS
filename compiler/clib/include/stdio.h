#ifndef _STDIO_H
#define _STDIO_H

/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: ANSI-C header file stdio.h
    Lang: english
*/
#include <stdarg.h>
#ifndef _SYS_TYPES_H
#   include <sys/types.h>
#endif

#ifndef NULL
#   ifdef __cplusplus
#	define NULL 0
#   else
#	define NULL (void*)0
#   endif /* __cplusplus */
#endif /* NULL */

#ifndef EOF
#   define EOF (-1)
#endif

#ifndef BUFSIZ
#   define BUFSIZ 1024
#endif

typedef struct __FILE
{
    void * fh;
    long   flags;
} FILE;

#define _STDIO_FILEFLAG_EOF	0x0001L
#define _STDIO_FILEFLAG_ERROR	0x0002L

extern FILE * stdin, * stdout, * stderr;

extern FILE * fopen (const char * name, const char * mode);
extern int fclose (FILE *);
extern int printf (const char * format, ...);
extern int vprintf (const char * format, va_list args);
extern int fprintf (FILE * fh, const char * format, ...);
extern int vfprintf (FILE * fh, const char * format, va_list args);
extern int fputc (int c, FILE * stream);
extern int fputs (const char * str, FILE * stream);
extern int puts (const char * str, FILE * stream);
extern int fflush (FILE * stream);
extern int fgetc (FILE * stream);
extern int ungetc (int c, FILE * stream);
extern char * fgets (char * buffer, int size, FILE * stream);
extern int feof (FILE * stream);
extern int ferror (FILE * stream);
extern void clearerr (FILE * stream);
extern size_t fread (void *ptr, size_t size, size_t nmemb, FILE * stream);
extern size_t fwrite (void *ptr, size_t size, size_t nmemb, FILE * stream);
extern int sprintf (char * str, const char * format, ...);
extern int vsprintf (char * str, const char * format, va_list args);
extern int snprintf (char * str, size_t n, const char * format, ...);
extern int vsnprintf (char * str, size_t n, const char * format, va_list args);


#ifdef AROS_ALMOST_COMPATIBLE
extern int __vcformat (void * data, int (*uc)(int, void *),
			const char * format, va_list args);
#endif

#define putc fputc
#define getc fgetc
#define getchar() fgetc(stdin)
#define gets(s) fgets(s, BUFSIZ, stdin)

#endif /* _STDIO_H */
