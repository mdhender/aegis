//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to manipulate ms
//
//
// M <text>
//
// A one-line message for the user.  Note that the format of <text>
// is not designed for machine parsing.  Although sometimes scripts
// and clients will have little choice, the exact text which is output
// is subject to vary at the discretion of the server and the example
// output given in this document is just that, example output.
//


#include <ac/string.h>

#include <output.h>
#include <response/m.h>
#include <response/private.h>


typedef struct response_m_ty response_m_ty;
struct response_m_ty
{
    response_ty     inherited;
    string_ty       *message;
};


static void
destructor(response_ty *rp)
{
    response_m_ty   *rep;

    rep = (response_m_ty *)rp;
    str_free(rep->message);
    rep->message = 0;
}


static void
write(response_ty *rp, output_ty *op)
{
    response_m_ty   *rep;
    const char      *cp;

    rep = (response_m_ty *)rp;
    cp = rep->message->str_text;
    for (;;)
    {
	const char      *ep;

	ep = strchr(cp, '\n');
	if (!ep)
	    break;
	output_fprintf(op, "M %.*s\n", (int)(ep - cp), cp);
	cp = ep + 1;
    }
    if (*cp)
	output_fprintf(op, "M %s\n", cp);
}


static const response_method_ty vtbl =
{
    sizeof(response_m_ty),
    destructor,
    write,
    response_code_M,
    0, // not flushable
};


response_ty *
response_m_new_v(const char *fmt, va_list ap)
{
    response_ty     *rp;
    response_m_ty   *rep;

    rp = response_new(&vtbl);
    rep = (response_m_ty *)rp;
    rep->message = str_vformat(fmt, ap);
    return rp;
}


response_ty *
response_m_new(const char *fmt, ...)
{
    va_list         ap;
    response_ty     *rp;

    va_start(ap, fmt);
    rp = response_m_new_v(fmt, ap);
    va_end(ap);
    return rp;
}