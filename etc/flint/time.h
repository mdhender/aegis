/*
 * Minimum correct ANSI C header.
 */
#ifndef ETC_FLINT_TIME_H
#define ETC_FLINT_TIME_H

#ifndef TYPEDEF_SIZE_T
#define TYPEDEF_SIZE_T
typedef unsigned long size_t;
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TYPEDEF_TIME_T
#define TYPEDEF_TIME_T
typedef long time_t;
#endif

#ifndef TYPEDEF_CLOCK_T
#define TYPEDEF_CLOCK_T
typedef unsigned long clock_t;
#endif

struct tm
{
  int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
  int tm_min;			/* Minutes.	[0-59] */
  int tm_hour;			/* Hours.	[0-23] */
  int tm_mday;			/* Day.		[1-31] */
  int tm_mon;			/* Month.	[0-11] */
  int tm_year;			/* Year	- 1900.  */
  int tm_wday;			/* Day of week.	[0-6] */
  int tm_yday;			/* Days in year.[0-365]	*/
  int tm_isdst;			/* DST.		[-1/0/1]*/
};

clock_t clock(void);
time_t time(time_t *);
double difftime(time_t, time_t);
time_t mktime(struct tm *);
size_t strftime(char *, size_t, const char *, const struct tm *);
char *strptime(const char *, const char *, struct tm *);
struct tm *gmtime(const time_t *);
struct tm *localtime(const time_t *);
char *asctime(const struct tm *);
char *ctime(const time_t *);

#endif /* ETC_FLINT_TIME_H */
