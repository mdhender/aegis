//
// aegis - project change supervisor
// Copyright (C) 2005-2010, 2012 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/sizeof.h>
#include <common/symtab.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/all.h>
#include <aede-policy/validation/authors.h>
#include <aede-policy/validation/debian-copyr.h>
#include <aede-policy/validation/description.h>
#include <aede-policy/validation/files/comments.h>
#include <aede-policy/validation/files/copyright.h>
#include <aede-policy/validation/files/crlf.h>
#include <aede-policy/validation/files/escapehyphen.h>
#include <aede-policy/validation/files/fsf_address.h>
#include <aede-policy/validation/files/gpl-version.h>
#include <aede-policy/validation/files/line_length.h>
#include <aede-policy/validation/files/merge-fhist.h>
#include <aede-policy/validation/files/merge-rcs.h>
#include <aede-policy/validation/files/no_tabs.h>
#include <aede-policy/validation/files/no_manifest.h>
#include <aede-policy/validation/files/printable.h>
#include <aede-policy/validation/files/reserved.h>
#include <aede-policy/validation/files/text.h>
#include <aede-policy/validation/files/vim-mode.h>
#include <aede-policy/validation/files/white_space.h>
#include <aede-policy/validation/man_pages.h>
#include <aede-policy/validation/version-info.h>


struct table_t
{
    const char *name;
    validation::pointer (*func)(void);
    bool all;
};

static table_t table[] =
{
    { "authors", validation_authors::create, false },
    { "debian/copyright", validation_debian_copyright::create, false },
    { "comments", validation_files_comments::create, false },
    { "copyright", validation_files_copyright::create, true },
    { "crlf", validation_files_crlf::create, true },
    { "description", validation_description::create, true },
    { "escape-hyphen", validation_files_escape_hyphen::create, true },
    { "fsf-address", validation_files_fsf_address::create, true },
    { "gpl-version", validation_files_gpl_version::create3, false },
    { "line-length", validation_files_line_length::create80, false },
    { "man-pages", validation_man_pages::create, false },
    { "merge-fhist", validation_files_merge_fhist::create, false },
    { "merge-rcs", validation_files_merge_rcs::create, false },
    { "no-manifest", validation_files_no_manifest::create, false },
    { "no-tabs", validation_files_no_tabs::create, false },
    { "printable", validation_files_printable::create, true },
    { "reserved", validation_files_reserved::create, false },
    { "reserved-words", validation_files_reserved::create, false },
    { "text", validation_files_text::create, false },
    { "version-info", validation_version_info::create, false },
    { "vim-mode", validation_files_vim_mode::create, false },
    { "white-space", validation_files_white_space::create, false },
};

static symtab_ty *stp;


validation::pointer
validation::factory(const char *cname)
{
    nstring name = nstring(cname).downcase();

    //
    // We don't put "all" in the table, otherwise we get an infinite
    // constructor loop.
    //
    if (name == nstring("all"))
        return validation_all::create();
    if (name.starts_with("line-length="))
        return validation_files_line_length::create(atoi(name.c_str() + 12));
    if (name.starts_with("gpl-version="))
        return validation_files_gpl_version::create(atoi(name.c_str() + 12));

    if (!stp)
    {
        stp = new symtab_ty(SIZEOF(table));
        for (table_t *tp = table; tp < ENDOF(table); ++tp)
            stp->assign(str_from_c(tp->name), tp);
    }
    string_ty *sname = str_from_c(cname);
    table_t *tp = (table_t *)stp->query(sname);
    if (!tp)
    {
        string_ty *other = stp->query_fuzzy(sname);
        if (other)
        {
            sub_context_ty sc;
            sc.var_set_string("Name", name);
            sc.var_set_string("Guess", other);
            sc.fatal_intl(i18n("no \"$name\", guessing \"$guess\""));
            // NOTREACHED
        }
        sub_context_ty sc;
        sc.var_set_string("Name", name);
        sc.fatal_intl(i18n("no $name list"));
        // NOTREACHED
    }
    str_free(sname);
    return tp->func();
}


void
validation::list(void)
{
    for (table_t *tp = table; tp < ENDOF(table); ++tp)
        printf("%s\n", tp->name);
}


void
validation::all(validation_list &where)
{
    for (table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (tp->all)
            where.push_back(tp->func());
    }
}


// vim: set ts=8 sw=4 et :
