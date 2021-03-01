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

#include <libaegis/os.h>
#include <common/str_list.h>
#include <common/nstring/list.h>


void
os_xargs(string_ty *the_command, string_list_ty *the_list, string_ty *dir)
{
    enum { chunk = 50 };
    size_t          j;

    for (j = 0; j < the_list->nstrings; j += chunk)
    {
	size_t          nargs;
	size_t          k;
	string_ty       *s;

	nargs = chunk;
	if (j + nargs > the_list->nstrings)
	    nargs = the_list->nstrings - j;
	string_list_ty wl;
	wl.push_back(the_command);
	for (k = 0; k < nargs; ++k)
	{
	    s = str_quote_shell(the_list->string[j + k]);
	    wl.push_back(s);
	    str_free(s);
	}
	s = wl.unsplit();
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT, dir);
	os_become_undo();
	str_free(s);
    }
}


void
os_xargs(const nstring &the_command, const nstring_list &the_list,
    const nstring &dir)
{
    enum { chunk = 50 };
    for (size_t j = 0; j < the_list.size(); j += chunk)
    {
	size_t nargs = chunk;
	if (j + nargs > the_list.size())
	    nargs = the_list.size() - j;
	nstring_list wl;
	wl.push_back(the_command);
	for (size_t k = 0; k < nargs; ++k)
	{
	    nstring s = the_list[j + k].quote_shell();
	    wl.push_back(s);
	}
	nstring s = wl.unsplit();
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT, dir);
	os_become_undo();
    }
}
