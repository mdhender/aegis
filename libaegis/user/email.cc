//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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
#include <common/ac/string.h>

#include <common/nstring/accumulator.h>
#include <libaegis/os/domain_name.h>
#include <libaegis/user.h>


/**
  * The clean_up function is used to clean up a user's full name
  * into an rfc822-acceptable string, suitable for use within an
  * email address.
  *
  * @param s
  *     The string to be cleaned up
  * @returns
  *     the cleaned up string
  */

static nstring
clean_up(const nstring &s)
{
    nstring_accumulator buf;
    unsigned char prev = ' ';
    for (const char *cp = s.c_str(); *cp; ++cp)
    {
	unsigned char c = *cp;
	if (isspace(c) || strchr("@,<>()[]{}'\"", c) || !isprint(c))
	    c = ' ';
	if (c != ' ' || prev != ' ')
	    buf.push_back(c);
	prev = c;
    }
    while (buf.size() > 0 && buf[buf.size() - 1] == ' ')
	buf.pop_back();
    return buf.mkstr();
}


string_ty *
user_email_address(user_ty::pointer up)
{
    static nstring result;
    result = up->get_email_address();
    return result.get_ref();
}


nstring
user_ty::get_email_address()
{
    uconf_ty *ucp = uconf_get();
    if (!ucp->email_address || !ucp->email_address->str_length)
    {
	//
	// Look for the domain name to go on the right hand side of the @ sign.
	//
	nstring domain = os_domain_name();

	//
	// Construct the email address.
	//
	nstring fnm = clean_up(full_name());
	ucp->email_address =
	    str_format
	    (
		"%s <%s@%s>",
		fnm.c_str(),
		login_name.c_str(),
		domain.c_str()
	    );
    }
    else
    {
	//
        // Even if the user has given us a string, we have to make sure
        // that it has all the right bits...
	//
	const char *s = ucp->email_address->str_text;
	const char *lt = strchr(s, '<');
	const char *at = strchr(s, '@');
	const char *gt = strchr(s, '>');
	if (lt && at && gt)
	{
	    // this is what we need, do nothing
	    if (lt == s)
	    {
		// Add their full name.
		nstring temp(ucp->email_address);
		nstring fnm = clean_up(full_name());
		ucp->email_address =
		    str_format("%s %s", fnm.c_str(), temp.c_str());
	    }
	}
	else if (at)
	{
	    // Add their full name.
	    nstring temp(ucp->email_address);
	    nstring fnm = clean_up(full_name());
	    ucp->email_address =
		str_format("%s <%s>", fnm.c_str(), temp.c_str());
	}
	else
	{
            //
            // Look for the domain name to go on the right hand side of
            // the @ sign.
	    //
	    nstring domain = os_domain_name();

	    //
	    // Construct the email address.
	    //
	    nstring fnm = clean_up(full_name());
	    nstring temp(ucp->email_address);
	    ucp->email_address =
		str_format
		(
		    "%s <%s@%s>",
		    fnm.c_str(),
		    temp.c_str(),
		    domain.c_str()
		);
	}
    }
    return nstring(ucp->email_address);
}
