//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate plurals
//

#include <common/ac/libintl.h>
#include <common/ac/string.h>

#include <common/language.h>
#include <common/stracc.h>
#include <libaegis/sub.h>
#include <libaegis/sub/plural_forms.h>
#include <libaegis/sub/plural_gram.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstr/list.h>


static char *header_begin;
static char *header_pos;


static int
header_getc(void)
{
    if (!header_pos || !*header_pos)
	return 0;
    return *header_pos++;
}


static void
header_ungetc(int c)
{
    if (!c)
	return;
    if (header_pos > header_begin && header_pos[-1] == c)
	--header_pos;
}


static string_ty *
header_get_line(void)
{
    static stracc_t sa;

    sa.clear();
    for (;;)
    {
	int             c;

	c = header_getc();
	if (!c)
	{
	    if (sa.empty())
		return 0;
	    break;
	}
	if (c == '\n')
	{
	    c = header_getc();
	    if (c != ' ' && c != '\t')
	    {
		header_ungetc(c);
		break;
	    }
	}
	sa.push_back(c);
    }
    return sa.mkstr();
}


static string_ty *
find_plural_forms(void)
{
    //
    // Find the relevant transaltion information.
    //
    language_human();
    header_pos = gettext("");
    language_C();
    header_begin = header_pos;

    //
    // Now parse out the relevant header information.
    //
    for (;;)
    {
	string_ty       *s;

	s = header_get_line();
	if (!s)
	    break;
	if (0 == strncasecmp(s->str_text, "plural-forms:", 13))
	{
	    string_ty       *result;

	    result = str_n_from_c(s->str_text + 13, s->str_length - 13);
	    str_free(s);
	    return result;
	}
	str_free(s);
    }

    //
    // No relevant information.  Use the default, which is suitable for
    // the Germanic family (Danish, Dutch, English, German, Norwegian
    // and Swedish), the Finno-Urgic family (Estonian and Finnish),
    // the Latin/Greek family (Greek), the Semitic family (Hebrew),
    // the Romanic family (Italian, Portugese and Spanish) and Esperanto.
    //
    return str_from_c("nplurals=2; plural=n!=1");
}


static string_ty *plural_forms;


wstring_ty *
sub_plural_forms(sub_context_ty *scp, wstring_list_ty *arg)
{
    trace(("sub_plural()\n{\n"));
    if (arg->size() < 2)
    {
	oh_dear:
	sub_context_error_set(scp, i18n("requires two or three arguments"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    //
    // This is undocumented: if the first argument starts with '@'
    // then use it instead of the Plural-Forms: header in the po/mo file.
    // It is used for testing via aesub(1).
    //
    size_t argpos = 1;
    if (arg->get(argpos)->wstr_text[0] == '@')
    {
	string_ty *s = wstr_to_str(arg->get(1));
	if (plural_forms)
	    str_free(plural_forms);
	plural_forms = str_n_from_c(s->str_text + 1, s->str_length - 1);
	str_free(s);
	++argpos;
    }
    else
    {
	if (!plural_forms)
	    plural_forms = find_plural_forms();
    }

    //
    // Get the number of items that the plural form is for.
    //
    if (argpos >= arg->size())
	goto oh_dear;
    string_ty *s = wstr_to_str(arg->get(argpos++));
    unsigned n = atoi(s->str_text);
    str_free(s);

    //
    // Run the expression tree extracted from the PO file header,
    // to determine which of the following arguments should be used.
    //
    n = sub_plural_gram(plural_forms, n);

    //
    // If the appropriately numbered argument is not present,
    // return the singular form.
    //
    if (argpos >= arg->size())
	goto oh_dear;
    if (argpos + n > arg->size())
	n = 0;
    wstring_ty *result = wstr_copy(arg->get(argpos + n));

    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
