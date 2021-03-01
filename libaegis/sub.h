/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1995, 1997 Peter Miller;
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

sub_context_ty *sub_context_New _((char *, int));
#define sub_context_new() sub_context_New(__FILE__, __LINE__)

void sub_context_delete _((sub_context_ty *));

void sub_var_clear _((sub_context_ty *));
void sub_var_set _((sub_context_ty *, char *name, char *fmt, ...));
void sub_var_optional _((sub_context_ty *, char *));
void sub_var_append_if_unused _((sub_context_ty *, char *));
void sub_var_override _((sub_context_ty *, char *));
void sub_var_resubstitute _((sub_context_ty *, char *));
void sub_errno_set _((sub_context_ty *));
void sub_errno_setx _((sub_context_ty *, int));
string_ty *substitute _((sub_context_ty *, struct change_ty *cp, string_ty *the_command));
struct string_ty *subst_intl _((sub_context_ty *, char *));
void subst_intl_project _((sub_context_ty *, struct project_ty *));
void subst_intl_change _((sub_context_ty *, struct change_ty *));

/*
 * internationalized error messages
 * (also the keywords necessary to pull out the msgid strings)
 */
void error_intl _((sub_context_ty *, char *));
void fatal_intl _((sub_context_ty *, char *));
void verbose_intl _((sub_context_ty *, char *));

/*
 * This macro does nothing by itself, but it serves as a keyword for the
 * xgettext program, when extracting internationalized msgid keys.
 */
#define i18n(x) (x)

/*
 * This function is only for use by functions within aegis/sub/ .c
 * files, when setting the error message.
 */
void sub_context_error_set _((sub_context_ty *, char *));

#endif /* SUB_H */
