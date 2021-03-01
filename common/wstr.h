//
//	aegis - project change supervisor
//	Copyright (C) 1995, 1998, 1999, 2004-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef COMMON_WSTR_H
#define COMMON_WSTR_H

#include <common/ac/stddef.h>
#include <common/ac/stdlib.h>
#include <common/main.h>

/** \addtogroup WString
  * \brief Wide char strings
  * \ingroup String
  * @{
  */

typedef unsigned long wstr_hash_ty;

struct wstring_ty
{
    wstr_hash_ty    wstr_hash;
    wstring_ty	    *wstr_next;
    long	    wstr_references;
    size_t	    wstr_length;
    wchar_t	    wstr_text[1];
};

#define wstr_n_from_c_release() wstr_n_from_c((const char *)0, 0)

void wstr_release(void);
wstring_ty *wstr_from_c(const char *);
wstring_ty *wstr_from_wc(const wchar_t *);
wstring_ty *wstr_n_from_c(const char *, size_t);
wstring_ty *wstr_n_from_wc(const wchar_t *, size_t);
wstring_ty *wstr_copy(wstring_ty *);
void wstr_free(wstring_ty *);
wstring_ty *wstr_catenate(const wstring_ty *, const wstring_ty *);
wstring_ty *wstr_cat_three(const wstring_ty *, const wstring_ty *,
    const wstring_ty *);
wstring_ty *wstr_to_upper(const wstring_ty *);
wstring_ty *wstr_to_lower(const wstring_ty *);
wstring_ty *wstr_capitalize(const wstring_ty *);
wstring_ty *wstr_to_ident(const wstring_ty *);
void wstr_to_mbs(const wstring_ty *, char **, size_t *);
int wstr_equal(const wstring_ty *, const wstring_ty *);

#ifndef DEBUG
#define wstr_equal(s1, s2) ((s1) == (s2))
#endif

struct string_ty;
struct string_ty *wstr_to_str(const wstring_ty *);
wstring_ty *str_to_wstr(const struct string_ty *);

wstring_ty *wstr_quote_shell(wstring_ty *);
int wstr_column_width(wstring_ty *);

/** @} */
#endif // COMMON_WSTR_H
