//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2006, 2008, 2009, 2012 Peter Miller
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

#include <common/str.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/base64.h>
#include <libaegis/input/cpio.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/input/uudecode.h>
#include <libaegis/os.h>
#include <libaegis/rfc822.h>
#include <libaegis/sub.h>

#include <aedist/open.h>


input_cpio *
aedist_open(const nstring &ifr, nstring *subject_p)
{
    //
    // Open the input file and verify the format.
    //
    os_become_orig();
    input ifp = input_file_open(ifr);
    rfc822 hdr;
    hdr.load(ifp, true);
    nstring s = hdr.get("mime-version");
    if (!s.empty())
    {
        s = hdr.get("content-type");
        if (s != "application/aegis-change-set")
            ifp->fatal_error("wrong content type");
    }

    //
    // Deal with the content encoding.
    //
    s = hdr.get("content-transfer-encoding");
    if (!s.empty())
    {
        static nstring base64("base64");
        static nstring uuencode("uuencode");

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
    // The contents we are interested in are
    // a gzipped cpio archive.
    //
    ifp = input_gunzip_open(ifp);
    ifp = input_bunzip2_open(ifp);
    input_cpio *cpio_p = new input_cpio(ifp);
    os_become_undo();

    //
    // Set the subject if they are interested.
    //
    if (subject_p)
    {
        s = hdr.get("subject");
        if (!s.empty())
            *subject_p = s;
        else
            *subject_p = "No Subject";
    }

    //
    // clean up and go home
    //
    return cpio_p;
}


// vim: set ts=8 sw=4 et :
