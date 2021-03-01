//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <libaegis/file.h>
#include <libaegis/os.h>
#include <libaegis/os/domain_name.h>
#include <libaegis/uname.h>


static bool
is_a_valid_domain_string(const nstring &candidate)
{
    const char *s = candidate.c_str();
    int numdots = 0;
    unsigned char state = '.';
    for (;;)
    {
	unsigned char c = *s++;
	switch (c)
	{
	case 0:
	    return (state == 0 && numdots != 0);

	case '.':
	    if (state == '.')
		return false;
	    state = '.';
	    ++numdots;
	    break;

	case '0': case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
	case 'v': case 'w': case 'x': case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
	case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
	case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
	case 'V': case 'W': case 'X': case 'Y': case 'Z':
	case '=':
	    state = 0;
	    break;

	default:
	    return false;
	}
    }
}


static bool
try_etc_mailname(nstring &result)
{
    bool ok = false;
    nstring mailname("/etc/mailname");
    os_become_orig();
    if (os_exists(mailname.get_ref()))
    {
	result = read_whole_file(mailname);
	ok = is_a_valid_domain_string(result);
    }
    os_become_undo();
    return ok;
}


static bool
try_a_command(const char *cmd, nstring &result)
{
    bool ok = false;
    nstring temp_file_name(os_edit_filename(0));
    nstring command(nstring(cmd) + " > " + temp_file_name + " 2> /dev/null");
    os_become_orig();
    nstring dir(os_curdir());
    int exit_status =
	os_execute_retcode
	(
	    command.get_ref(),
	    OS_EXEC_FLAG_SILENT,
	    dir.get_ref()
	);
    if (exit_status == 0)
    {
	result = read_whole_file(temp_file_name);
	ok = is_a_valid_domain_string(result);
    }
    os_unlink_errok(temp_file_name);
    os_become_undo();
    return ok;
}


static bool
try_hostname_d(nstring &result)
{
    return try_a_command("hostname -d", result);
}


static bool
try_dnsdomainname(nstring &result)
{
    return try_a_command("dnsdomainname", result);
}


static bool
try_domainname(nstring &result)
{
    return try_a_command("domainname", result);
}


nstring
os_domain_name(void)
{
    static nstring cached_answer;
    if (cached_answer.size() == 0)
    {
	if
	(
	    !try_etc_mailname(cached_answer)
	&&
	    !try_hostname_d(cached_answer)
	&&
	    !try_dnsdomainname(cached_answer)
	&&
	    !try_domainname(cached_answer)
	)
	    cached_answer = nstring(uname_node_get());
    }
    return cached_answer;
}
