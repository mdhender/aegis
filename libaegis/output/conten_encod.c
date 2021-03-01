/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001-2003 Peter Miller;
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
 * MANIFEST: functions to manipulate conten_encods
 */

#include <arglex.h>
#include <output/conten_encod.h>
#include <output/base64.h>
#include <output/quoted_print.h>
#include <output/uuencode.h>
#include <sub.h>


content_encoding_t
content_encoding_grok(const char *name)
{
    typedef struct table_t table_t;
    struct table_t
    {
	const char    *name;
	content_encoding_t value;
    };

    static table_t table[] =
    {
	{ "None", content_encoding_none, },
	{ "8Bit", content_encoding_none, },
	{ "Base64", content_encoding_base64, },
	{ "Quoted_Printable", content_encoding_quoted_printable, },
	{ "Unix_to_Unix_encode", content_encoding_uuencode, },
    };

    table_t         *tp;
    sub_context_ty  *scp;

    /*
     * Look for the name in the table.
     */
    for (tp = table; tp < ENDOF(table); ++tp)
    {
	if (arglex_compare(tp->name, name))
	    return tp->value;
    }

    /*
     * It's a fatal error of the name is not found.
     */
    scp = sub_context_new();
    sub_var_set_charstar(scp, "Name", name);
    fatal_intl(scp, i18n("content transfer encoding $name unknown"));
    sub_context_delete(scp);
    return content_encoding_none;
}


void
content_encoding_header(output_ty *ofp, content_encoding_t name)
{
    switch (name)
    {
    case content_encoding_unset:
    case content_encoding_none:
	break;

    case content_encoding_base64:
	output_fputs(ofp, "Content-Transfer-Encoding: base64\n");
	break;

    case content_encoding_quoted_printable:
	output_fputs(ofp, "Content-Transfer-Encoding: quoted-printable\n");
	break;

    case content_encoding_uuencode:
	output_fputs(ofp, "Content-Transfer-Encoding: uuencode\n");
	break;
    }
}


output_ty *
output_content_encoding(output_ty *ofp, content_encoding_t name)
{
    switch (name)
    {
    case content_encoding_unset:
    case content_encoding_none:
	break;

    case content_encoding_base64:
	return output_base64(ofp, 1);

    case content_encoding_quoted_printable:
	return output_quoted_printable(ofp, 1, 0);

    case content_encoding_uuencode:
	return output_uuencode(ofp, 1);
    }
    return ofp;
}
