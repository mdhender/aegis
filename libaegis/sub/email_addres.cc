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

#include <common/arglex.h>
#include <common/nstring/list.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/email_addres.h>
#include <libaegis/user.h>


wstring
sub_email_address(sub_context_ty *, const wstring_list &arg)
{
    trace(("sub_email_address()\n{\n"));
    nstring_list words;
    bool quote = false;
    const char *separator = 0;
    size_t j = 1;
    for (; j < arg.size(); ++j)
    {
	nstring option = arg[j].to_nstring();
	if (arglex_compare("-Comma", option.c_str(), 0))
	    separator = ", ";
	else if (arglex_compare("-Quote", option.c_str(), 0))
	    quote = true;
	else
	    break;
    }
    for (; j < arg.size(); ++j)
    {
	nstring login = arg[j].to_nstring();
	user_ty::pointer up = user_ty::create(login);
	nstring s = up->get_email_address();
	if (quote)
            s = s.quote_shell();
        words.push_back_unique(s);
    }

    //
    // Convert the work list to a single string.
    //
    nstring rs = words.unsplit(separator);
    wstring result(rs);

    //
    // all done
    //
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
