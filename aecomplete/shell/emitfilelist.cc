//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
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

#include <aecomplete/shell.h>
#include <common/str_list.h>
#include <common/symtab.h>


static size_t
how_much_in_common(string_ty *s1, string_ty *s2)
{
    size_t          n;
    const char      *cp1;
    const char      *cp2;

    for
    (
	cp1 = s1->str_text, cp2 = s2->str_text, n = 0;
	*cp1 && *cp2;
	++n, ++cp1, ++cp2
    )
    {
	if (*cp1 != *cp2)
	    return n;
    }
    return n;
}


static string_ty *
following_slash(string_ty *s, size_t len)
{
    for (;;)
    {
	if (s->str_length <= len)
	    return str_copy(s);
	if (s->str_text[len] == '/')
	    return str_n_from_c(s->str_text, len + 1);
	++len;
    }
}


void
shell_emit_file_list(shell_ty *sh, string_list_ty *slp)
{
    symtab_ty       *stp;
    size_t          j;
    size_t          longest;

    //
    // Give up if nothing matches.
    //
    if (slp->nstrings == 0)
	return;

    //
    // Look for longest common prefix.
    //
    longest = slp->string[0]->str_length;
    for (j = 1; j < slp->nstrings; ++j)
    {
	size_t          n;

	n = how_much_in_common(slp->string[0], slp->string[j]);
	if (n < longest)
	    longest = n;
    }

    //
    // Rewrite the file names to only use from the longest common prefix
    // to the following slash (or end of string if no slash).
    // Use the symtab to make it an O(n) operation.
    //
    stp = symtab_alloc(slp->nstrings);
    for (j = 0; j < slp->nstrings; ++j)
    {
	string_ty       *s;

	s = following_slash(slp->string[j], longest);
	if (symtab_query(stp, s))
	{
	    str_free(s);
	    continue;
	}
	shell_emit(sh, s);
	symtab_assign(stp, s, s);
	str_free(s);
    }
    symtab_free(stp);
}
