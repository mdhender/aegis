//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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
#include <common/ac/stdio.h>

#include <common/nstring.h>
#include <common/str.h>
#include <libaegis/change.h>
#include <libaegis/emit/brief_descri.h>
#include <libaegis/http.h>


void
emit_change_brief_description(change::pointer cp)
{
    enum { default_midpoint = 20 };
    string_ty *s = change_brief_description_get(cp);
    if (!s)
	return;
    if (s->str_length <= default_midpoint)
    {
	emit_change_href(cp, "menu");
	html_encode_string(s);
	printf("</a>");
	return;
    }

    //
    // Work backwards looking for a space to stop the link at like the
    // first line in a book chapter bolds the first line or the first
    // few words.
    //
    size_t midpoint = default_midpoint;
    while (midpoint > 0 && isalnum((unsigned char)s->str_text[midpoint - 1]))
	--midpoint;
    while (midpoint > 0 && !isalnum((unsigned char)s->str_text[midpoint - 1]))
	--midpoint;
    if (midpoint == 0)
    {
	//
        // Work forwards looking for a space to stop the link at like
        // the first line in a book chapter bolds the first line or the
        // first few words.
	//
	midpoint = default_midpoint;
	while
	(
	    midpoint < s->str_length
	&&
	    midpoint < 2 * default_midpoint
	&&
	    isalnum((unsigned char)s->str_text[midpoint])
	)
	    ++midpoint;
	if (midpoint >= 2 * default_midpoint)
	{
	    //
	    // Nothing seems to work, just split at the default place.
	    //
	    midpoint = default_midpoint;
	}
    }

    nstring left(s->str_text, midpoint);
    nstring right(s->str_text + midpoint, s->str_length - midpoint);
    emit_change_href(cp, "menu");
    html_encode_string(left);
    printf("</a>");
    html_encode_string(right);
}
