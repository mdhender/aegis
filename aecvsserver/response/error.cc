//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: functions to manipulate errors
//
//
// error <error-code> <text>
//
// The command completed with an error.  <errno-code> is a symbolic
// error code (e.g. ENOENT); if the server doesn't support this feature,
// or if it's not appropriate for this particular message, it just omits
// the <errno-code> (in that case there are two spaces after 'error').
// Text is an error message such as that provided by strerror(), or
// any other message the server wants to use.  The <text> is like the
// M response, in the sense that it is not particularly intended to be
// machine-parsed; servers may wish to print an error message with MT
// responses, and then issue a 'error' response without <text> (although
// it should be noted that MT currently has no way of flagging the output
// as intended for standard error, the way that the E response does).
//

#include <ac/string.h>

#include <error.h> // for assert
#include <output.h>
#include <response/error.h>
#include <response/private.h>


struct response_error_ty
{
    response_ty     inherited;
    string_ty       *message;
    string_ty       *extra;
};


static void
destructor(response_ty *rp)
{
    response_error_ty *rep;

    rep = (response_error_ty *)rp;
    str_free(rep->message);
    rep->message = 0;
    if (rep->extra)
    {
	str_free(rep->extra);
	rep->extra = 0;
    }
}


static void
write(response_ty *rp, output_ty *op)
{
    response_error_ty *rep;
    const char      *lhs;
    const char      *cp;

    rep = (response_error_ty *)rp;
    lhs = rep->extra ? rep->extra->str_text : "";
    cp = rep->message->str_text;
    for (;;)
    {
	const char      *ep;

	ep = strchr(cp, '\n');
	if (!ep)
	    break;
	op->fprintf("E %.*s\n", (int)(ep - cp), cp);
	cp = ep + 1;
    }
    op->fprintf("error %s %s\n", lhs, cp);
}


static const response_method_ty vtbl =
{
    sizeof(response_error_ty),
    destructor,
    write,
    response_code_error,
    1, // flushable
};


response_ty *
response_error_new_v(const char *fmt, va_list ap)
{
    response_ty     *rp;
    response_error_ty *rep;

    assert(fmt);
    rp = response_new(&vtbl);
    rep = (response_error_ty *)rp;
    rep->message = str_vformat(fmt, ap);
    rep->extra = 0;
    return rp;
}


response_ty *
response_error_new(const char *fmt, ...)
{
    va_list         ap;
    response_ty     *rp;

    assert(fmt);
    va_start(ap, fmt);
    rp = response_error_new_v(fmt, ap);
    va_end(ap);
    return rp;
}


void
response_error_extra(response_ty *rp, const char *extra)
{
    response_error_ty *rep;

    if (rp->vptr != &vtbl)
	return;
    rep = (response_error_ty *)rp;
    rep->extra = str_from_c(extra);
}
