//
//      aegis - project change supervisor
//      Copyright (C) 1991-1994, 1999, 2002, 2005, 2006, 2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef TRACE_H
#define TRACE_H

#include <common/ac/stddef.h>

#include <common/gcc-attribute.h>
#include <common/debug.h>

struct string_ty; // forward

/** \addtogroup Trace
  * \brief Trace capability support
  * \ingroup Common
  * @{
  */

#ifdef DEBUG
#define trace_pretest_ \
        (                                                       \
                (                                               \
                        trace_pretest_result                    \
                ?                                               \
                        trace_pretest_result                    \
                :                                               \
                        trace_pretest(__FILE__, &trace_pretest_result) \
                )                                               \
        &                                                       \
                1                                               \
        )
#define trace_where_ trace_where(__FILE__, __LINE__)
#define trace(x) (void)(trace_pretest_ && (trace_where_, trace_printf x, 0))
#define trace_if() (trace_pretest_ && (trace_where_, 1))
#else
#define trace(x)
#define trace_if() 0
#endif

//
// This variable is static to each file which
// includes the "trace.h" file.
// Tracing is file-by-file, but need only test this once.
// Files will fail to trace if a trace call is executed in them
// prior to a call to trace_enable turning it on.
//
#ifdef DEBUG
static int trace_pretest_result;
#endif

int trace_pretest(const char *file, int *result);
void trace_where(const char *file, int line);
void trace_printf(const char *, ...) ATTR_PRINTF(1, 2);
const char * trace_args();
void trace_enable(const char *);
void trace_indent_reset(void);


#if defined(__STDC__) || defined(__stdc__)
#define trace_stringize(x) #x
#else
#define trace_stringize(x) "x"
#endif

void trace_bool_real(const char *name, const bool &value);
void trace_char_real(const char *, const char *);
void trace_char_unsigned_real(const char *, const unsigned char *);
void trace_int_real(const char *, const int *);
void trace_int_unsigned_real(const char *, const unsigned *);
void trace_long_real(const char *, const long *);
void trace_long_unsigned_real(const char *, const unsigned long *);
void trace_pointer_real(const char *, const void *);
void trace_short_real(const char *, const short *);
void trace_short_unsigned_real(const char *, const unsigned short *);
class nstring; // forward
void trace_string_real(const char *, const nstring &);
void trace_string_real(const char *, const string_ty *);
void trace_string_real(const char *, const char *);
void trace_time_real(const char *, long);
void trace_double_real(const char *, const double &);

#ifdef DEBUG

#define trace_bool(x)                                           \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_bool_real(trace_stringize(x), (x)),           \
            0                                                   \
        )                                                       \
    )

#define trace_char(x)                                           \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_char_real(trace_stringize(x), &(x)),          \
            0                                                   \
        )                                                       \
    )

#define trace_char_unsigned(x)                                  \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_char_unsigned_real(trace_stringize(x), &(x)), \
            0                                                   \
        )                                                       \
    )

#define trace_int(x)                                            \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_int_real(trace_stringize(x), &(x)),           \
            0                                                   \
        )                                                       \
    )

#define trace_int_unsigned(x)                                   \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_int_unsigned_real(trace_stringize(x), &(x)),  \
            0                                                   \
        )                                                       \
    )

#define trace_long(x)                                           \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_long_real(trace_stringize(x), &(x)),          \
            0                                                   \
        )                                                       \
    )

#define trace_long_unsigned(x)                                  \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_long_unsigned_real(trace_stringize(x), &(x)), \
            0                                                   \
        )                                                       \
    )

#define trace_pointer(x)                                        \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_pointer_real(trace_stringize(x), &(x)),       \
            0                                                   \
        )                                                       \
    )

#define trace_short(x)                                          \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_short_real(trace_stringize(x), &(x)),         \
            0                                                   \
        )                                                       \
    )

#define trace_short_unsigned(x)                                 \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_short_unsigned_real(trace_stringize(x), &(x)), \
            0                                                   \
        )                                                       \
    )

#define trace_string(x)                                         \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_string_real(trace_stringize(x), (x)),         \
            0                                                   \
        )                                                       \
    )

#define trace_nstring(x)                                        \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_string_real(trace_stringize(x), (x).c_str()), \
            0                                                   \
        )                                                       \
    )

#define trace_time(x)                                           \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_time_real(trace_stringize(x), (long)(x)),     \
            0                                                   \
        )                                                       \
    )

#define trace_double(x)                                         \
    (void)                                                      \
    (                                                           \
        trace_pretest_                                          \
    &&                                                          \
        (                                                       \
            trace_where_,                                       \
            trace_time_real(trace_stringize(x), (x)),           \
            0                                                   \
        )                                                       \
    )

#else

#define trace_bool(x)
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
#define trace_nstring(x)
#define trace_time(x)
#define trace_double(x)

#endif

/**
  * The unctrl function may be used to turn a character into something
  * printable for debugging purposes.  It's very verbose.
  */
const char *unctrl(int);

/** @} */
#endif // TRACE_H
// vim: set ts=8 sw=4 et :
