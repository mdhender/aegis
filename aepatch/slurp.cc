//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
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

#include <common/ac/stdlib.h>

#include <common/error.h>
#include <libaegis/input.h>
#include <libaegis/input/base64.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/crlf.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/input/quoted_print.h>
#include <libaegis/input/uudecode.h>
#include <libaegis/os.h>
#include <libaegis/rfc822.h>
#include <libaegis/sub.h>
#include <libaegis/zero.h>

#include <aepatch/slurp.h>


patch_list_ty *
patch_slurp(string_ty *ifn)
{
    //
    // open the input
    //
    os_become_orig();
    input ifp = input_file_open(ifn);
    bool is_remote = ifp->is_remote();

    //
    // Look for an RFC822 header.
    //
    // It could tell us useful things, like the content transfer
    // encoding, and the project name.
    //
    rfc822 hdr;
    hdr.load(ifp, true);

    //
    // Deal with the content encoding.
    //
    nstring s = hdr.get("content-transfer-encoding");
    if (!s.empty())
    {
	static nstring base64("base64");
	static nstring uuencode("uuencode");
	static nstring quotprin("quoted-printable");
	static nstring eightbit("8bit");
	static nstring sevenbit("7bit");
	static nstring none("none");

	//
	// We could cope with other encodings here,
	// if we ever need to.
	//
	if (s == base64)
	{
	    //
	    // The rest of the input is in base64 encoding.
	    //
	    ifp = new input_base64(ifp);
	}
	else if (s == uuencode)
	{
	    //
	    // The rest of the input is uuencoded.
	    //
	    ifp = new input_uudecode(ifp);
	}
	else if (s == quotprin)
	{
	    //
	    // The rest of the input is uuencoded.
	    //
	    ifp = new input_quoted_printable(ifp);
	}
	else if (s == sevenbit || s == eightbit || s == none)
	{
	    // do nothing
	}
	else
	{
	    sub_context_ty sc;
	    sc.var_set_string("Name", s);
	    string_ty *tmp =
		sc.subst_intl(i18n("content transfer encoding $name unknown"));
	    ifp->fatal_error(tmp->str_text);
	    str_free(tmp);
	}
    }

    //
    // The contents could be compressed.
    //
    ifp = input_gunzip_open(ifp);
    ifp = input_bunzip2_open(ifp);

    //
    // Filter out any CRLF sequences.
    //
    ifp = new input_crlf(ifp);

    //
    // Read the patch body.
    //
    patch_list_ty *plp = patch_read(ifp, 1);
    ifp.close();
    os_become_undo();
    assert(plp);
    if (is_remote)
	plp->comment = str_copy(ifn);

    //
    // Pull useful information out of the patch header.
    //
    s = hdr.get("x-aegis-project-name");
    if (!s.empty())
	plp->project_name = str_copy(s.get_ref());

    s = hdr.get("x-aegis-change-number");
    if (!s.empty())
    {
	char *endptr = 0;
	long n = strtol(s.c_str(), &endptr, 0);
	if (!*endptr && n >= 0)
	    plp->change_number = magic_zero_encode(n);
    }

    s = hdr.get("subject");
    if (!s.empty())
	plp->brief_description = str_copy(s.get_ref());

    s = hdr.get("date");
    if (!s.empty())
    {
	if (!plp->description)
    	    plp->description = str_from_c("No description.");
	string_ty *s2 =
	    str_format("Date: %s\n%s", s.c_str(), plp->description->str_text);
	str_free(plp->description);
	plp->description = s2;
    }

    s = hdr.get("from");
    if (!s.empty())
    {
	if (!plp->description)
    	    plp->description = str_from_c("No description.");
	string_ty *s2 =
	    str_format("From: %s\n%s", s.c_str(), plp->description->str_text);
	str_free(plp->description);
	plp->description = s2;
    }

    //
    // all done
    //
    return plp;
}
