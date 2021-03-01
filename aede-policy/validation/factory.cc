//
//      aegis - project change supervisor
//      Copyright (C) 2005-2008 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/symtab.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/all.h>
#include <aede-policy/validation/description.h>
#include <aede-policy/validation/files/copyright.h>
#include <aede-policy/validation/files/crlf.h>
#include <aede-policy/validation/files/fsf_address.h>
#include <aede-policy/validation/files/gpl-version.h>
#include <aede-policy/validation/files/line_length.h>
#include <aede-policy/validation/files/merge-fhist.h>
#include <aede-policy/validation/files/merge-rcs.h>
#include <aede-policy/validation/files/no_tabs.h>
#include <aede-policy/validation/files/no_manifest.h>
#include <aede-policy/validation/files/printable.h>
#include <aede-policy/validation/files/text.h>
#include <aede-policy/validation/files/white_space.h>


static validation *
new_validation_copyright()
{
    return new validation_files_copyright();
}


static validation *
new_validation_files_crlf()
{
    return new validation_files_crlf();
}


static validation *
new_validation_description()
{
    return new validation_description();
}


static validation *
new_validation_files_fsf_address()
{
    return new validation_files_fsf_address();
}


static validation *
new_validation_files_gpl_version()
{
    return new validation_files_gpl_version();
}


static validation *
new_validation_files_line_length()
{
    return new validation_files_line_length();
}


static validation *
new_validation_files_merge_fhist()
{
    return new validation_files_merge_fhist();
}


static validation *
new_validation_files_merge_rcs()
{
    return new validation_files_merge_rcs();
}


static validation *
new_validation_files_no_manifest()
{
    return new validation_files_no_manifest();
}


static validation *
new_validation_files_no_tabs()
{
    return new validation_files_no_tabs();
}


static validation *
new_validation_files_printable()
{
    return new validation_files_printable();
}


static validation *
new_validation_files_text()
{
    return new validation_files_text();
}


static validation *
new_validation_files_white_space()
{
    return new validation_files_white_space();
}


struct table_t
{
    const char *name;
    validation *(*func)(void);
    bool all;
};

static table_t table[] =
{
    { "copyright", new_validation_copyright, true },
    { "crlf", new_validation_files_crlf, true },
    { "description", new_validation_description, true },
    { "fsf-address", new_validation_files_fsf_address, true },
    { "gpl-version", new_validation_files_gpl_version, false },
    { "line-length", new_validation_files_line_length, false },
    { "merge-fhist", new_validation_files_merge_fhist, false },
    { "merge-rcs", new_validation_files_merge_rcs, false },
    { "no-manifest", new_validation_files_no_manifest, false },
    { "no-tabs", new_validation_files_no_tabs, false },
    { "printable", new_validation_files_printable, true },
    { "text", new_validation_files_text, false },
    { "white-space", new_validation_files_white_space, false },
};

static symtab_ty *stp;


validation *
validation::factory(const char *cname)
{
    nstring name = nstring(cname).downcase();

    //
    // We don't put "all" in the table, otherwise we get an infinite
    // constructor loop.
    //
    if (name == nstring("all"))
        return new validation_all();
    if (name.starts_with("line-length="))
        return new validation_files_line_length(atoi(name.c_str() + 12));
    if (name.starts_with("gpl-version="))
        return new validation_files_gpl_version(atoi(name.c_str() + 12));

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
validation::list()
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
