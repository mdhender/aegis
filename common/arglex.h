//
//	aegis - project change supervisor
//	Copyright (C) 1991-1993, 1995, 1999, 2001-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface definition for common/arglex.c
//

#ifndef ARGLEX_H
#define ARGLEX_H

/** \addtogroup arglex
  * \brief Parse command switch
  * \ingroup Common
  * @{
  */
#include <main.h>

#define ARGLEX_END_MARKER { (const char *)0, 0, }

enum
{
    ARGLEX_PREFETCH_FAIL = -99,
    arglex_token_eoln = -20,
    arglex_token_help,
    arglex_token_number,
    arglex_token_number_incomplete,
    arglex_token_option,
    arglex_token_option_incomplete,
    arglex_token_page_length,
    arglex_token_page_width,
    arglex_token_stdio,
    arglex_token_string,
    arglex_token_string_incomplete,
    arglex_token_trace,
    arglex_token_version
};

struct arglex_table_ty
{
    const char      *t_name;
    int             t_token;
};

struct arglex_value_ty
{
    const char      *alv_string;
    long            alv_number;
};

extern int      arglex_token;
extern arglex_value_ty arglex_value;

void arglex_init(int, char **, arglex_table_ty *);
int arglex(void);
int arglex_prefetch(int *, int);

/**
  * The arglex_compare function is used to compare a command line string
  * with a formal spec of the option, to see if they compare equal.
  *
  * The actual is case-insensitive.  Uppercase in the formal means a
  * mandatory character, while lower case means optional.  Any number of
  * consecutive optional characters may be supplied by actual, but none
  * may be skipped, unless all are skipped to the next non-lower-case
  * letter.
  *
  * The underscore (_) is like a lower-case minus, it matches "", "-"
  * and "_".
  *
  * The "*" in a pattern matches everything to the end of the line,
  * anything after the "*" is ignored.  The rest of the line is pointed
  * to by the "partial" variable as a side-effect (else it will be 0).
  * This rather ugly feature is to support "-I./dir" type options.
  *
  * A backslash in a pattern nominates an exact match required, case
  * must matche excatly here.  This rather ugly feature is to support
  * "-I./dir" type options.
  *
  * For example: "-project" and "-P' both match "-Project", as does
  * "-proJ", but "-prj" does not.
  *
  * For example: "-devDir" and "-d_d' both match
  * "-Development_Directory", but "-dvlpmnt_drctry" does not.
  *
  * For example: to match include path specifications, use a pattern
  * such as "-\\I*", and the partial global variable will have the path
  * in it on return.
  *
  * \param formal
  *     the "pattern" for the option
  * \param actual
  *     what the user supplied
  * \param partial
  *     Where to put the results of a "*" match.
  *
  * \returns
  *     int; zero if no match, non-zero if they do match.
  */
bool arglex_compare(const char *formal, const char *actual,
    const char **partial);

const char *arglex_token_name(int);

arglex_table_ty *arglex_table_catenate(arglex_table_ty *, arglex_table_ty *);


struct arglex_dispatch_ty
{
    int             token;
    void            (*func)(void);
    int             priority;
};

void arglex_dispatch(arglex_dispatch_ty *table, int table_len,
    void (*usage)(void));

void arglex_synthetic(int, char **, int);
void arglex_retable(arglex_table_ty *);
int arglex_get_string(void);

/** @} */
#endif // ARGLEX_H
