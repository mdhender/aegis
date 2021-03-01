/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 1995, 1997, 1999, 2002 Peter Miller;
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
 * MANIFEST: interface definition for aegis/sub.c
 */

#ifndef SUB_H
#define SUB_H

#include <str.h>

struct change_ty; /* existence */
struct project_ty; /* existence */
typedef struct sub_context_ty sub_context_ty;

sub_context_ty *sub_context_New(const char *, int);
#define sub_context_new() sub_context_New(__FILE__, __LINE__)

void sub_context_delete(sub_context_ty *);

void sub_var_clear(sub_context_ty *);
void sub_var_set_format(sub_context_ty *, const char *, const char *, ...);
void sub_var_set_string(sub_context_ty *, const char *, struct string_ty *);
void sub_var_set_charstar(sub_context_ty *, const char *, const char *);
void sub_var_set_long(sub_context_ty *, const char *, long);
void sub_var_optional(sub_context_ty *, const char *);
void sub_var_append_if_unused(sub_context_ty *, const char *);
void sub_var_override(sub_context_ty *, const char *);
void sub_var_resubstitute(sub_context_ty *, const char *);
void sub_errno_set(sub_context_ty *);
void sub_errno_setx(sub_context_ty *, int);
string_ty *substitute(sub_context_ty *, struct change_ty *cp,
	string_ty *the_command);
string_ty *substitute_p(sub_context_ty *, struct project_ty *cp,
	string_ty *the_command);
struct string_ty *subst_intl(sub_context_ty *, const char *);
void subst_intl_project(sub_context_ty *, struct project_ty *);
void subst_intl_change(sub_context_ty *, struct change_ty *);

/*
 * internationalized error messages
 * (also the keywords necessary to pull out the msgid strings)
 */
void error_intl(sub_context_ty *, const char *);
void fatal_intl(sub_context_ty *, const char *) NORETURN;
void verbose_intl(sub_context_ty *, const char *);

/*
 * This macro does nothing by itself, but it serves as a keyword for the
 * xgettext program, when extracting internationalized msgid keys.
 */
#define i18n(x) (x)

/*
 * This function is only for use by functions within aegis/sub/ .c
 * files, when setting the error message.
 */
void sub_context_error_set(sub_context_ty *, const char *);
struct project_ty *sub_context_project_get(sub_context_ty *);
struct change_ty *sub_context_change_get(sub_context_ty *);

#endif /* SUB_H */
