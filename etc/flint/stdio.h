#ifndef ETC_FLINT_STDIO_H
#define ETC_FLINT_STDIO_H

#ifndef TYPEDEF_SIZE_T
#define TYPEDEF_SIZE_T
typedef unsigned long size_t;
#endif

#ifndef TYPEDEF_FILE
#define TYPEDEF_FILE
typedef struct FILE FILE;
#endif /* TYPEDEF_FILE */

typedef struct fpos_t fpos_t;

#ifndef NULL
#define NULL 0
#endif

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#define BUFSIZ 1024

#define EOF (-1)

#define FOPEN_MAX 1024

#define FILENAME_MAX 1024

#define L_tmpnam 20

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0

#define TMP_MAX 10

extern FILE *stderr;
extern FILE *stdin;
extern FILE *stdout;

int remove(const char *);
int rename(const char *, const char *);
FILE *tmpfile(void);
char *tmpnam(char *);
int fclose(FILE *);
int fflush(FILE *);
FILE *fopen(const char *, const char *);
FILE *fdopen(int, const char *);
FILE *freopen(const char *, const char *, FILE *);
void setbuf(FILE *, char *);
int setvbuf(FILE *, char *, int, size_t);

int fprintf(FILE *, const char *, ...);
int fscanf(FILE *, const char *, ...);
int printf(const char *, ...);
int scanf(const char *, ...);
int sprintf(char *, const char *, ...);
int sscanf(char *, const char *, ...);

#ifndef TYPEDEF_VA_ALIST
#define TYPEDEF_VA_ALIST
typedef void *va_alist;
#endif /* TYPEDEF_VA_ALIST */

int vfprintf(FILE *, const char *, va_alist);
int vprintf(const char *, va_alist);
int vsprintf(char *, const char *, va_alist);

int fgetc(FILE *);
char *fgets(char *, int, FILE *);
int fputc(int, FILE *);
char *fputs(char *, FILE *);
int getc(FILE *);
int getchar(void);
char *gets(char *);
int putc(int, FILE *);
int putchar(int);
int puts(char *);
int ungetc(int, FILE *);
size_t fread(void *, size_t, size_t, FILE *);
size_t fwrite(const void *, size_t, size_t, FILE *);
int fgetpos(FILE *, fpos_t *);
int fseek(FILE *, long, int);
int fsetpos(FILE *, const fpos_t *);
long ftell(FILE *);
void rewind(FILE *);
void clearerr(FILE *);
int feof(FILE *);
int ferror(FILE *);
void perror(const char *);

#endif /* ETC_FLINT_STDIO_H */
