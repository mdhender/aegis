//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004-2006, 2008 Peter Miller
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

#include <common/ac/ctype.h>

#include <common/language.h>
#include <common/str.h>
#include <common/stracc.h>


string_ty *
str_capitalize(string_ty *s)
{
    static stracc_t sa;
    size_t          j;
    int             prev_was_alpha;

    language_human();
    prev_was_alpha = 0;
    sa.clear();
    for (j = 0; j < s->str_length; ++j)
    {
	char            c;

	c = s->str_text[j];
	if (islower(c))
	{
	    if (!prev_was_alpha)
		c = toupper(c);
	    prev_was_alpha = 1;
	}
	else if (isupper(c))
	{
	    if (prev_was_alpha)
		c = tolower(c);
	    prev_was_alpha = 1;
	}
	else
	    prev_was_alpha = 0;
	sa.push_back(c);
    }
    language_C();
    return sa.mkstr();
}
