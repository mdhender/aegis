/*
 * MANIFEST: minimal stdlib.h substitute
 */

#ifndef _STDLIB_H
#define _STDLIB_H

typedef struct div_t div_t;
struct div_t
{
    int quot;
    int rem;
};

typedef struct ldiv_t ldiv_t;
struct ldiv_t
{
    long quot;
    long rem;
};

double atof();
int atoi();
long atol();
double strtod();
unsigned long strtoul();
int rand();
void srand();
void *calloc();
void free();
void *malloc();
void *realloc();
void abort();
int atexit();
void exit();
char *getenv();
int system();
void *bsearch();
void qsort();
int abs();
div_t div();
long labs();
ldiv_t ldiv();

#endif /* _STDLIB_H */
