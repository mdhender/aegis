/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 1999, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for common/trace.c
 */


#ifndef TRACE_H
#define TRACE_H

#include <ac/stddef.h>

#include <main.h>

/** \addtogroup Trace
  * \brief Trace capability support
  * \ingroup Common
  * @{
  */

#ifdef DEBUG
#define trace_pretest_ \
	(							\
		(						\
			trace_pretest_result			\
		?						\
			trace_pretest_result			\
		:						\
			trace_pretest(__FILE__, &trace_pretest_result) \
		)						\
	&							\
		1						\
	)
#define trace_where_ trace_where(__FILE__, __LINE__)
#define trace(x) (void)(trace_pretest_ && (trace_where_, trace_printf x, 0))
#define trace_if() (trace_pretest_ && (trace_where_, 1))
#else
#define trace(x)
#define trace_if() 0
#endif

/*
 * This variable is static to each file which
 * includes the "trace.h" file.
 * Tracing is file-by-file, but need only test this once.
 * Files will fail to trace if a trace call is executed in them
 * prior to a call to trace_enable turning it on.
 */
#ifdef DEBUG
static int trace_pretest_result;
#endif

int trace_pretest(const char *file, int *result);
void trace_where(const char *file, int line);
void trace_printf(const char *, ...) ATTR_PRINTF(1, 2);
void trace_enable(const char *);
void trace_indent_reset(void);


#if defined(__STDC__) || defined(__stdc__)
#define trace_stringize(x) #x
#else
#define trace_stringize(x) "x"
#endif

void trace_char_real(const char *, const char *);
void trace_char_unsigned_real(const char *, const unsigned char *);
void trace_int_real(const char *, const int *);
void trace_int_unsigned_real(const char *, const unsigned *);
void trace_long_real(const char *, const long *);
void trace_long_unsigned_real(const char *, const unsigned long *);
void trace_pointer_real(const char *, const void *);
void trace_short_real(const char *, const short *);
void trace_short_unsigned_real(const char *, const unsigned short *);
void trace_string_real(const char *, const char *);

#ifdef DEBUG

#define trace_char(x)						\
	(void)							\
	(							\
		trace_pretest_					\
	&&							\
		(						\
			trace_where_,				\
			trace_char_real(trace_stringize(x), &x), \
			0					\
		)						\
	)

#define trace_char_unsigned(x)					\
	(void)							\
	(							\
		trace_pretest_					\
	&&							\
		(						\
			trace_where_,				\
			trace_char_unsigned_real(trace_stringize(x), &x), \
			0					\
		)						\
	)

#define trace_int(x)						\
	(void)							\
	(							\
		trace_pretest_					\
	&&							\
		(						\
			trace_where_,				\
			trace_int_real(trace_stringize(x), &x),	\
			0					\
		)						\
	)

#define trace_int_unsigned(x)					\
	(void)							\
	(							\
		trace_pretest_					\
	&&							\
		(						\
			trace_where_,				\
			trace_int_unsigned_real(trace_stringize(x), &x), \
			0					\
		)						\
	)

#define trace_long(x)						\
	(void)							\
	(							\
		trace_pretest_					\
	&&							\
		(						\
			trace_where_,				\
			trace_long_real(trace_stringize(x), &x), \
			0					\
		)						\
	)

#define trace_long_unsigned(x)					\
	(void)							\
	(							\
		trace_pretest_					\
	&&							\
		(						\
			trace_where_,				\
			trace_long_unsigned_real(trace_stringize(x), &x), \
			0					\
		)						\
	)

#define trace_pointer(x)					\
	(void)							\
	(							\
		trace_pretest_					\
	&&							\
		(						\
			trace_where_,				\
			trace_pointer_real(trace_stringize(x), &x), \
			0					\
		)						\
	)

#define trace_short(x)						\
	(void)							\
	(							\
		trace_pretest_					\
	&&							\
		(						\
			trace_where_,				\
			trace_short_real(trace_stringize(x), &x), \
			0					\
		)						\
	)

#define trace_short_unsigned(x)					\
	(void)							\
	(							\
		trace_pretest_					\
	&&							\
		(						\
			trace_where_,				\
			trace_short_unsigned_real(trace_stringize(x), &x), \
			0					\
		)						\
	)

#define trace_string(x)						\
	(void)							\
	(							\
		trace_pretest_					\
	&&							\
		(						\
			trace_where_,				\
			trace_string_real(trace_stringize(x), x), \
			0					\
		)						\
	)

#else

#define trace_char(x)
#define trace_char_unsigned(x)
#define trace_int(x)
#define trace_int_unsigned(x)
#define trace_long(x)
#define trace_long_unsigned(x)
#define trace_pointer(x)
#define trace_short(x)
#define trace_short_unsigned(x)
#define trace_string(x)

#endif

/**
  * The unctrl function may be used to turn a character into something
  * printable for debugging purposes.  It's very verbose.
  */
const char *unctrl(int);

/** @} */
#endif /* TRACE_H */
