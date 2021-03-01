//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: implementation of the validation_factory class
//

#include <common/ac/stdio.h>

#include <common/symtab.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/all.h>
#include <aede-policy/validation/copyright.h>
#include <aede-policy/validation/crlf.h>
#include <aede-policy/validation/description.h>
#include <aede-policy/validation/printable.h>
#include <aede-policy/validation/text.h>


static validation *
new_validation_copyright()
{
    return new validation_copyright();
}


static validation *
new_validation_crlf()
{
    return new validation_crlf();
}


static validation *
new_validation_description()
{
    return new validation_description();
}


static validation *
new_validation_printable()
{
    return new validation_printable();
}


static validation *
new_validation_text()
{
    return new validation_text();
}


struct table_t
{
    const char *name;
    validation *(*func)(void);
};

static table_t table[] =
{
    { "copyright", new_validation_copyright },
    { "crlf", new_validation_crlf },
    { "description", new_validation_description },
    { "text", new_validation_text },
    { "printable", new_validation_printable },
};

static symtab_ty *stp;


validation *
validation::factory(const char *name)
{
    //
    // We don't put "all" in the table, otherwise we get an infinite
    // constructor loop.
    //
    if (nstring(name) == nstring("all"))
	return new validation_all();

    if (!stp)
    {
	stp = new symtab_ty(SIZEOF(table));
	for (table_t *tp = table; tp < ENDOF(table); ++tp)
	    stp->assign(str_from_c(tp->name), tp);
    }
    string_ty *sname = str_from_c(name);
    table_t *tp = (table_t *)stp->query(sname);
    if (!tp)
    {
	string_ty *other = stp->query_fuzzy(sname);
	if (other)
	{
	    sub_context_ty sc;
	    sc.var_set_charstar("Name", name);
	    sc.var_set_string("Guess", other);
	    sc.fatal_intl(i18n("no \"$name\", guessing \"$guess\""));
	    // NOTREACHED
	}
	sub_context_ty sc;
	sc.var_set_charstar("Name", name);
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
	// The "printable" validation includes the "text" validation.
	if (nstring(tp->name) != "text")
	    where.push_back(tp->func());
    }
}
