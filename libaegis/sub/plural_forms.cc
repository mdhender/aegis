//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <common/ac/libintl.h>
#include <common/ac/string.h>

#include <common/language.h>
#include <common/nstring/accumulator.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/plural_forms.h>
#include <libaegis/sub/plural_gram.h>


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


static nstring
header_get_line(void)
{
    static nstring_accumulator sa;
    sa.clear();
    for (;;)
    {
	int c = header_getc();
	if (!c)
	{
	    if (sa.empty())
		return "";
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


static nstring
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
	nstring s = header_get_line();
	if (!s)
	    break;
	if (0 == strncasecmp(s.c_str(), "plural-forms:", 13))
	{
	    return nstring(s.c_str() + 13, s.size() - 13);
	}
    }

    //
    // No relevant information.  Use the default, which is suitable for
    // the Germanic family (Danish, Dutch, English, German, Norwegian
    // and Swedish), the Finno-Urgic family (Estonian and Finnish),
    // the Latin/Greek family (Greek), the Semitic family (Hebrew),
    // the Romanic family (Italian, Portugese and Spanish) and Esperanto.
    //
    return "nplurals=2; plural=n!=1";
}


static nstring plural_forms;


wstring
sub_plural_forms(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_plural()\n{\n"));
    wstring result;
    if (arg.size() < 2)
    {
	oh_dear:
	scp->error_set(i18n("requires two or three arguments"));
	trace(("}\n"));
	return result;
    }

    //
    // This is undocumented: if the first argument starts with '@'
    // then use it instead of the Plural-Forms: header in the po/mo file.
    // It is used for testing via aesub(1).
    //
    size_t argpos = 1;
    if (arg[argpos][0] == '@')
    {
	nstring s = arg[1].to_nstring();
	plural_forms = s.substring(1, s.size());
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
    if (argpos >= arg.size())
	goto oh_dear;
    nstring s = arg[argpos++].to_nstring();
    unsigned n = s.to_long();

    //
    // Run the expression tree extracted from the PO file header,
    // to determine which of the following arguments should be used.
    //
    n = sub_plural_gram(plural_forms, n);

    //
    // If the appropriately numbered argument is not present,
    // return the singular form.
    //
    if (argpos >= arg.size())
	goto oh_dear;
    if (argpos + n > arg.size())
	n = 0;
    result = arg[argpos + n];

    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
