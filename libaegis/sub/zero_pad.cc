//
//      aegis - project change supervisor
//      Copyright (C) 1996, 2003-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <libaegis/sub.h>
#include <libaegis/sub/zero_pad.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <common/mem.h>


wstring
sub_zero_pad(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_zero_pad()\n{\n"));
    trace(("arg.size() => %d\n", int(arg.size())));
    if (arg.size() != 3)
    {
	scp->error_set(i18n("requires two arguments"));
	trace(("}\n"));
	return wstring();
    }
    long n = arg[2].to_nstring().to_long();
    trace(("n = %ld;\n", n));

    wstring result;
    if (n <= (long)arg[1].size())
	result = arg[1];
    else
    {
	size_t len = n - arg[1].size();
	nstring s3 = arg[1].to_nstring();
	char *buffer = new char [len + 1];
	memset(buffer, '0', len);
	buffer[len] = 0;
	nstring s2 = nstring::format("%s%s", buffer, s3.c_str());
	delete [] buffer;
	trace(("s2 = %s;\n", s2.quote_c().c_str()));
	result = wstring(s2);
    }
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
