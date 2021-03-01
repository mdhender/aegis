//
// aegis - project change supervisor
// Copyright (C) 2001, 2002, 2004-2006, 2008-2010, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <libaegis/input.h>
#include <libaegis/input/base64.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/crlf.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/input/multipart.h>
#include <libaegis/input/quoted_print.h>
#include <libaegis/input/uudecode.h>
#include <libaegis/os.h>
#include <libaegis/rfc822.h>
#include <libaegis/sub.h>
#include <libaegis/zero.h>

#include <aepatch/slurp.h>


static nstring
extract_boundary(const nstring &s)
{
    // Content-Type: multipart/alternative; boundary="some-string"
    nstring_list parts;
    parts.split(s, ";", true);
    for (size_t j = 0; j < parts.size(); ++j)
    {
        nstring part = parts[j];
        if (part.starts_with("boundary="))
        {
            nstring s2 = part.substr(9, part.size() - 9);
            if (s2.front() == '"' && s2.back() == '"')
                s2 = s2.substr(1, s2.size() - 2);
            if (!s2.empty())
                return s2;
        }
    }
    // failed to find a boundary string
    return nstring();
}


static void
copy_useful_stuff(patch_list_ty *plp, rfc822 &hdr)
{
    //
    // Pull useful information out of the patch header.
    //
    nstring s = hdr.get("x-aegis-project-name");
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
    else
    {
        if (plp->description)
        {
            // There could be one in there already
            const char *cp =
                strstr(plp->description->str_text, "New Revision: ");
            if (cp)
            {
                cp += 14;
                char *endptr = 0;
                long n = strtol(cp, &endptr, 10);
                if (endptr != cp && n > 0)
                    plp->change_number = n;
            }
        }
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
}


static patch_list_ty *
patch_slurp(input ifp)
{
    //
    // Look for an RFC822 header.
    //
    // It could tell us useful things, like the content transfer
    // encoding, and the project name.
    //
    rfc822 hdr;
    hdr.load(ifp, true);

    //
    // Watch out for multi-part
    //
    // Content-Type: multipart/alternative;  boundary="some-string"
    //
    nstring ct = hdr.get("content-type");
    if (ct.starts_with("multipart/"))
    {
        nstring boundary = extract_boundary(ct);
        if (!boundary.empty())
        {
            ifp = input_multipart::create(ifp, boundary);
            patch_list_ty *plp = patch_slurp(ifp);
            copy_useful_stuff(plp, hdr);
            return plp;
        }
    }

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
            // The rest of the input is quoted-printable.
            //
            ifp = input_quoted_printable::create(ifp);
        }
        else if (s == eightbit)
        {
            //
            // M$ Exchange Server "generously" takes plain-text emails
            // and turns them into multipart/alternative message
            // containing two parts: the original text and then HTML.
            //
            // Unfortunately, it discards the Content-Type of the
            // original message, and attaches "Content-Type: 8bit" to
            // the part containing the original email content.
            //
            // If the original Content-Type was actually
            // quoted-printable, we attempt to recover.
            //
            if (input_quoted_printable::recognise(ifp))
                ifp = input_quoted_printable::create(ifp);
        }
        else if (s == sevenbit || s == none)
        {
            // do nothing
        }
        else
        {
            sub_context_ty sc;
            sc.var_set_string("Name", s.quote_c());
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
    ifp = input_crlf::create(ifp);

    //
    // Read the patch body.
    //
    patch_list_ty *plp = patch_read(ifp, 1);
    assert(plp);

    copy_useful_stuff(plp, hdr);

    //
    // all done
    //
    return plp;
}


patch_list_ty *
patch_slurp(string_ty *ifn)
{
    //
    // open the input
    //
    os_become_orig();
    input ifp = input_file_open(ifn);
    patch_list_ty *plp = patch_slurp(ifp);
    if (plp && ifp->is_remote())
        plp->comment = str_copy(ifn);
    ifp.close();
    os_become_undo();
    return plp;
}


// vim: set ts=8 sw=4 et :
