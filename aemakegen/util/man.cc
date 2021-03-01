//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <common/nstring/list.h>
#include <common/trace.h>
#include <libaegis/locale_name.h>

#include <aemakegen/util.h>


bool
looks_like_a_man_page(const nstring &filename)
{
    trace(("looks_like_a_man_page(filename = %s)\n{\n",
        filename.quote_c().c_str()));
    nstring stem = extract_man_page_details(filename);
    trace(("stem = %s;\n", stem.quote_c().c_str()));
    trace(("return %d;\n", !stem.empty()));
    trace(("}\n"));
    return !stem.empty();
}


static nstring
extract_man_page_details(const nstring_list &part, size_t idx)
{
    trace(("%s\n{\n", __PRETTY_FUNCTION__));

    //
    // Patterns used here...
    //     man[1-8nl]/{progname}.{ext}
    //     */man[1-8nl]/{progname}.{ext}
    //
    if (idx + 2 == part.size() && part[idx].gmatch("man[1-8nl]"))
    {
        nstring section = part[idx].substr(3, 1);
        nstring x = "." + section;
        const char *s = part[idx + 1].c_str();
        const char *extn = strstr(s, x.c_str());
        if (extn && extn > s)
        {
            // Note: we don't add "man/" to the front, because the
            // $(mandir) make variable includes the "man/" part already.
            nstring result = (part[idx] + "/" + part[idx + 1]);
            trace(("return %s;\n}\n", result.quote_c().c_str()));
            return result;
        }
    }

    //
    // No details to be extracted.
    //
    trace(("return \"\";\n}\n"));
    return nstring();
}


nstring
extract_man_page_details(const nstring &filename)
{
    trace(("extract_man_page_details(filename = %s)\n{\n",
        filename.quote_c().c_str()));
    nstring_list part;
    part.split(filename, "/");

    //
    // File name patterns...
    //     {progname}/{progname}.man
    //     {progname}/{progname}.1
    //     {progname}/{progname}.8
    //     src/{progname}/{progname}.man
    //     src/{progname}/{progname}.1
    //     src/{progname}/{progname}.8
    //
    // Assume "en" locale.
    //
    if (part.size() >= 2)
    {
        size_t idx = part.size() - 2;
        if (part[idx + 1].starts_with(part[idx] + "."))
        {
            nstring ext = part[idx + 1].substr(part[idx].size() + 1, 9999);
            if (ext == "1" || ext == "8")
            {
                // Note: we don't add "man/" to the front, because the
                // $(mandir) make variable includes the "man/" part already.
                nstring result = ("man" + ext + "/" + part[idx] + "." + ext);
                trace(("return %s;\n}\n", result.quote_c().c_str()));
                return result;
            }
            if (ext == "man")
            {
                nstring result = ("man1/" + part[idx] + ".1");
                trace(("return %s;\n}\n", result.quote_c().c_str()));
                return result;
            }
        }
    }

    //
    // If there is a directory component name that is a valid locale
    // name, then install the man page into
    //
    //     $(mandir)/{locale}/manN/name.N,
    //
    // except if the locale is "en" in which case install the man page into
    //
    //     $(mandir)/manN/name.N
    //
    // File name patterns...
    //     man/fr/man1/fred.1
    //     lib/ru/man1/fred.1
    //     mandir/es_BR/man1/fred.1
    //
    if (part.size() >= 4)
    {
        nstring locale_name = part[part.size() - 3];
        if (is_a_locale_name(locale_name))
        {
            nstring details = extract_man_page_details(part, part.size() - 2);
            if (!details.empty())
            {
                if (locale_name != "en")
                    details = locale_name + "/" + details;
                trace(("return %s;\n}\n", details.quote_c().c_str()));
                return details;
            }
        }
    }

    //
    // File name patterns...
    //     man/man1/fred.1
    //     mandir/man1/fred.1
    //
    // Assume "en" locale.
    //
    if (part.size() >= 3)
    {
        nstring details = extract_man_page_details(part, part.size() - 2);
        if (!details.empty())
        {
            trace(("return %s;\n}\n", details.quote_c().c_str()));
            return details;
        }
    }

    //
    // File name patterns...
    //     man1/fred.1
    //
    // Assume "en" locale.
    //
    if (part.size() == 2)
    {
        nstring details = extract_man_page_details(part, 0);
        if (!details.empty())
        {
            trace(("return %s;\n}\n", details.quote_c().c_str()));
            return details;
        }
    }

    //
    // No man page details found.
    //
    trace(("return \"\";\n}\n"));
    return nstring();
}


// vim: set ts=8 sw=4 et :
