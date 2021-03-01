//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
// //
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

#include <common/ac/string.h>

#include <libaegis/output.h>
#include <aecvsserver/response/error.h>


response_error::~response_error()
{
    str_free(message);
    message = 0;
    if (extra_text)
    {
        str_free(extra_text);
        extra_text = 0;
    }
}


response_error::response_error(string_ty *arg1, string_ty *arg2) :
    message(arg1),
    extra_text(arg2)
{
}


void
response_error::write(output::pointer op)
{
    const char *lhs = extra_text ? extra_text->str_text : "";
    const char *cp = message->str_text;
    for (;;)
    {
        const char *ep = strchr(cp, '\n');
        if (!ep)
            break;
        op->fprintf("E %.*s\n", (int)(ep - cp), cp);
        cp = ep + 1;
    }
    op->fprintf("error %s %s\n", lhs, cp);
}


response_code_ty
response_error::code_get()
    const
{
    return response_code_error;
}


bool
response_error::flushable()
    const
{
    return true;
}


void
response_error::extra(string_ty *arg)
{
    extra_text = arg;
}


// vim: set ts=8 sw=4 et :
