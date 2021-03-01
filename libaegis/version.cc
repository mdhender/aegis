//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1999, 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate versions
//


#include <ac/stdio.h>

#include <arglex2.h>
#include <help.h>
#include <output/pager.h>
#include <output/wrap.h>
#include <progname.h>
#include <quit.h>
#include <sub.h>
#include <trace.h>
#include <version.h>
#include <version_stmp.h>


void
version_copyright()
{
    static const char *const text[] =
    {
	"All rights reserved.\n",
	"\n",
	"The %s program comes with ABSOLUTELY NO WARRANTY; ",
	"for details use the '%s -VERSion License' command.  ",
	"The %s program is free software, and you are welcome ",
	"to redistribute it under certain conditions; for ",
	"details use the '%s -VERSion License' command.\n",
    };

    output_ty	    *fp;
    const char	    *const *cpp;
    const char	    *progname;

    progname = progname_get();
    fp = output_pager_open();
    fp = output_wrap_open(fp, 1, -1);
    fp->fprintf("%s version %s\n", progname, version_stamp());
    fp->fprintf("Copyright (C) %s Peter Miller;\n", copyright_years());
    for (cpp = text; cpp < ENDOF(text); ++cpp)
	fp->fprintf(*cpp, progname);
    delete fp;
}


static void
version_license(void)
{
    help("aelic", (void (*)(void))0);
}


static void
version_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -VERSion [ <info-name> ]\n", progname);
    fprintf(stderr, "       %s -VERSion -Help\n", progname);
    quit(1);
}


struct table_ty
{
    const char      *name;
    void            (*func)(void);
};


static table_ty table[] =
{
    { "Copyright",	version_copyright,	},
    { "License",	version_license,	},
};


static void
version_main(void)
{
    sub_context_ty  *scp;
    void            (*func)(void);
    const char      *name;

    trace(("version_main()\n{\n"));
    arglex();
    name = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(version_usage);
	    continue;

	case arglex_token_string:
	    if (name)
	    {
	       	scp = sub_context_new();
	       	fatal_intl(scp, i18n("too many info names"));
	    }
	    name = arglex_value.alv_string;
	    break;
	}
	arglex();
    }

    if (name)
    {
	int		nhit;
	table_ty	*tp;
	string_ty	*s1;
	string_ty	*s2;
	table_ty	*hit[SIZEOF(table)];
	int		j;

	nhit = 0;
	for (tp = table; tp < ENDOF(table); ++tp)
	{
	    if (arglex_compare(tp->name, name, 0))
	       	hit[nhit++] = tp;
	}
	switch (nhit)
	{
	case 0:
	    scp = sub_context_new();
	    sub_var_set_charstar(scp, "Name", name);
	    fatal_intl(scp, i18n("no info $name"));
	    // NOTWEACHED

	case 1:
	    break;

	default:
	    s1 = str_from_c(hit[0]->name);
	    for (j = 1; j < nhit; ++j)
	    {
	       	s2 = str_format("%s, %s", s1->str_text, hit[j]->name);
	       	str_free(s1);
	       	s1 = s2;
	    }
	    scp = sub_context_new();
	    sub_var_set_charstar(scp, "Name", name);
	    sub_var_set_string(scp, "Name_List", s1);
	    fatal_intl(scp, i18n("info $name ambig ($name_list)"));
	    // NOTREACHED
	}
	arglex();
	func = hit[0]->func;
    }
    else
	func = version_copyright;

    func();
    trace(("}\n"));
}


static void
version_help(void)
{
    help("aev", version_usage);
}


void
version(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, version_help, },
    };

    trace(("version()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), version_main);
    trace(("}\n"));
}
