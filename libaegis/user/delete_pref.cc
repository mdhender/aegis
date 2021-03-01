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

#include <common/ac/stdio.h>
#include <common/ac/unistd.h>

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/user.h>


enum del_pref
{
    del_pref_unset,
    del_pref_keep_not,
    del_pref_interactive,
    del_pref_keep
};

static del_pref cmd_line_pref = del_pref_unset;


void
user_ty::delete_file_argument(void (*usage)(void))
{
    if (cmd_line_pref != del_pref_unset)
    {
	mutually_exclusive_options3
	(
	    arglex_token_keep_not,
	    arglex_token_interactive,
	    arglex_token_keep,
	    usage
	);
    }
    switch (arglex_token)
    {
    default:
	assert(0);

    case arglex_token_keep:
	cmd_line_pref = del_pref_keep;
	break;

    case arglex_token_interactive:
	cmd_line_pref = del_pref_interactive;
	break;

    case arglex_token_keep_not:
	cmd_line_pref = del_pref_keep_not;
	break;
    }
}


static bool
ask(const nstring &filename, bool isdir)
{
    struct table_ty
    {
	const char      *name;
	del_pref        set;
	bool            result;
    };

    //
    // The order of items in the table needs to be considered
    // carefully.  The "unset" items must come before the "set"
    // items; particularly those with similar names.  Do not sort
    // this table alphabetically.
    //
    static table_ty table[] =
    {
	{ "No", del_pref_unset, false, },
	{ "False", del_pref_unset, false, },
	{ "Never", del_pref_keep, false, },
	{ "None", del_pref_keep, false, },
	{ "Yes", del_pref_unset, true, },
	{ "True", del_pref_unset, true, },
	{ "All", del_pref_keep_not, true, },
	{ "Always", del_pref_keep_not, true, },
    };
    table_ty	    *tp;
    char	    buffer[100];
    char	    *bp;
    int		    c;

    for (;;)
    {
	printf
	(
	    "Delete the %s %s? ",
	    filename.quote_c().c_str(),
	    (isdir ? "directory and everything below it" : "file")
	);
	fflush(stdout);

	bp = buffer;
	for (;;)
	{
	    c = getchar();
	    if (c == '\n' || c == EOF)
		break;
	    if (bp < ENDOF(buffer) - 1)
		*bp++ = c;
	}
	*bp = 0;

	for (tp = table; tp < ENDOF(table); ++tp)
	{
	    if (arglex_compare(tp->name, buffer, 0))
	    {
		if (tp->set != del_pref_unset)
		    cmd_line_pref = tp->set;
		return tp->result;
	    }
	}
	printf("Please answer 'yes', 'no', 'all' or 'none'.\n");
    }
}


bool
user_ty::delete_file_query(const nstring &filename, bool isdir,
    int default_preference)
{
    //
    // if the preference was not set on the command line,
    // read it fron the user config file
    //
    trace(("user_ty::delete_file_query(this = %08lX, filename = %s, "
	"isdir = %d, dflt = %d)\n{\n", (long)this, filename.quote_c().c_str(),
        isdir, default_preference));
    if (cmd_line_pref == del_pref_unset)
    {
	trace(("mark\n"));
	if (default_preference > 0)
	{
	    // delete, keep not
	    cmd_line_pref = del_pref_keep_not;
	}
	else if (default_preference == 0)
	{
	    // delete not, keep
	    cmd_line_pref = del_pref_keep;
	}
	else
	{
	    uconf_ty *ucp = uconf_get();
	    switch (ucp->delete_file_preference)
	    {
	    default:
		cmd_line_pref = del_pref_keep_not;
		break;

	    case uconf_delete_file_preference_interactive:
		cmd_line_pref = del_pref_interactive;
		break;

	    case uconf_delete_file_preference_keep:
		cmd_line_pref = del_pref_keep;
		break;
	    }
	}
    }

    //
    // if the preference is to ask but we are in the background,
    // delete the files without asking.
    //
    if
    (
	cmd_line_pref == del_pref_interactive
    &&
	(!isatty(0) || os_background())
    )
    {
	trace(("mark\n"));
	cmd_line_pref = del_pref_keep_not;
    }

    //
    // figure the result
    //
    bool result = true;
    switch (cmd_line_pref)
    {
    case del_pref_unset:
    case del_pref_keep_not:
	break;

    case del_pref_interactive:
	result = ask(filename, isdir);
	break;

    case del_pref_keep:
	result = false;
	break;
    }
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}
