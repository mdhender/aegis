//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2002-2008 Peter Miller
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

#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <common/arglex.h>
#include <aefind/function/stat.h>
#include <aefind/shorthand/stat.h>
#include <common/str.h>
#include <libaegis/sub.h>
#include <aefind/tree.h>
#include <aefind/tree/arithmetic.h>
#include <aefind/tree/constant.h>
#include <aefind/tree/list.h>
#include <aefind/tree/now.h>
#include <aefind/tree/relative.h>
#include <aefind/tree/this.h>


tree::pointer
shorthand_atime(diadic_t cmp, int n, int units)
{
    // now
    tree::pointer tp1 = tree_now_new();

    // {}
    tree::pointer tp2 = tree_this::create();

    // atime({})
    tree::pointer tp3 = tree_atime::create(tp2);

    // now - atime({})
    tree::pointer tp4 = tree_subtract::create(tp1, tp3);

    // units
    rpt_value::pointer vp5 = rpt_value_integer::create(units);
    tree::pointer tp5 = tree_constant::create(vp5);

    // (now - atime({})) / units
    tree::pointer tp6 = tree_divide::create(tp4, tp5);

    rpt_value::pointer vp7 = rpt_value_integer::create(n);
    tree::pointer tp7 = tree_constant::create(vp7);

    // (now - atime({})) / units == n
    return cmp(tp6, tp7);
}


tree::pointer
shorthand_ctime( diadic_t cmp, int n, int units)
{
    // now
    tree::pointer tp1 = tree_now_new();

    // {}
    tree::pointer tp2 = tree_this::create();

    // ctime({})
    tree::pointer tp3 = tree_ctime::create(tp2);

    // now - ctime({})
    tree::pointer tp4 = tree_subtract::create(tp1, tp3);

    // units
    rpt_value::pointer vp5 = rpt_value_integer::create(units);
    tree::pointer tp5 = tree_constant::create(vp5);

    // (now - ctime({})) / units
    tree::pointer tp6 = tree_divide::create(tp4, tp5);

    rpt_value::pointer vp7 = rpt_value_integer::create(n);
    tree::pointer tp7 = tree_constant::create(vp7);

    // (now - ctime({})) / units == n
    return cmp(tp6, tp7);
}


tree::pointer
shorthand_gid(diadic_t cmp, int n)
{
    // {}
    tree::pointer tp1 = tree_this::create();

    // gid({})
    tree::pointer tp2 = tree_gid::create(tp1);

    // n
    rpt_value::pointer vp3 = rpt_value_integer::create(n);
    tree::pointer tp3 = tree_constant::create(vp3);

    // gid({}) == n
    return cmp(tp2, tp3);
}


tree::pointer
shorthand_ino(diadic_t cmp, int n)
{
    // {}
    tree::pointer tp1 = tree_this::create();

    // ino({})
    tree::pointer tp2 = tree_ino::create(tp1);

    // n
    rpt_value::pointer vp3 = rpt_value_integer::create(n);
    tree::pointer tp3 = tree_constant::create(vp3);

    // ino({}) == n
    return cmp(tp2, tp3);
}


tree::pointer
shorthand_mode(int n)
{
    // {}
    tree::pointer tp1 = tree_this::create();

    // mode({})
    tree::pointer tp2 = tree_mode::create(tp1);

    // n
    rpt_value::pointer vp3 = rpt_value_integer::create(n);
    tree::pointer tp3 = tree_constant::create(vp3);

    // mode({}) == n
    return tree_eq::create(tp2, tp3);
}


tree::pointer
shorthand_mtime(diadic_t cmp, int n, int units)
{
    // now
    tree::pointer tp1 = tree_now_new();

    // {}
    tree::pointer tp2 = tree_this::create();

    // mtime({})
    tree::pointer tp3 = tree_mtime::create(tp2);

    // now - mtime({})
    tree::pointer tp4 = tree_subtract::create(tp1, tp3);

    // units
    rpt_value::pointer vp5 = rpt_value_integer::create(units);
    tree::pointer tp5 = tree_constant::create(vp5);

    // (now - mtime({})) / units
    tree::pointer tp6 = tree_divide::create(tp4, tp5);

    // n
    rpt_value::pointer vp7 = rpt_value_integer::create(n);
    tree::pointer tp7 = tree_constant::create(vp7);

    // (now - mtime({})) / units == n
    return cmp(tp6, tp7);
}


tree::pointer
shorthand_newer(const nstring &filename)
{
    // {}
    tree::pointer tp1 = tree_this::create();

    // mtime({})
    tree::pointer tp2 = tree_mtime::create(tp1);

    // filename
    rpt_value::pointer vp3 = rpt_value_string::create(filename);
    tree::pointer tp3 = tree_constant::create(vp3);

    // mtime(filename)
    tree::pointer tp4 = tree_mtime::create(tp3);

    // mtime({}) > mtime(filename)
    return tree_gt::create(tp2, tp4);
}


tree::pointer
shorthand_nlink(diadic_t cmp, int n)
{
    // {}
    tree::pointer tp1 = tree_this::create();

    // nlink({})
    tree::pointer tp2 = tree_nlink::create(tp1);

    // n
    rpt_value::pointer vp3 = rpt_value_integer::create(n);
    tree::pointer tp3 = tree_constant::create(vp3);

    // nlink({}) == n
    return cmp(tp2, tp3);
}


tree::pointer
shorthand_size(diadic_t cmp, int n)
{
    // {}
    tree::pointer tp1 = tree_this::create();

    // size({})
    tree::pointer tp2 = tree_size::create(tp1);

    // n
    rpt_value::pointer vp3 = rpt_value_integer::create(n);
    tree::pointer tp3 = tree_constant::create(vp3);

    // size({}) == n
    return cmp(tp2, tp3);
}


tree::pointer
shorthand_uid(diadic_t cmp, int n)
{
    // {}
    tree::pointer tp1 = tree_this::create();

    // uid({})
    tree::pointer tp2 = tree_uid::create(tp1);

    // n
    rpt_value::pointer vp3 = rpt_value_integer::create(n);
    tree::pointer tp3 = tree_constant::create(vp3);

    // uid({}) == n
    return cmp(tp2, tp3);
}


static nstring
type_name_by_pattern(const nstring &abbrev)
{
    struct table_ty
    {
	const char      *pattern;
	const char      *name;
    };

    static table_ty table[] =
    {
	{ "Block",		"block_special",	},
	{ "Block_Special",	"block_special",	},
	{ "Character",		"character_special",	},
	{ "Character_Special",	"character_special",	},
	{ "Directory",		"directory",		},
	{ "File",		"file",			},
	{ "Link",		"symbolic_link",	},
	{ "Named_Pipe",		"named_pipe",		},
	{ "First_In_First_Out",	"named_pipe",		},
	{ "Normal",		"file",			},
	{ "Pipe",		"named_pipe",		},
	{ "Plain",		"file",			},
	{ "Socket",		"socket",		},
	{ "Symbolic_Link",	"symbolic_link",	},
    };

    for (table_ty *tp = table; tp < ENDOF(table); ++tp)
	if (arglex_compare(tp->pattern, abbrev.c_str(), 0))
    	    return tp->name;
    sub_context_ty sc;
    sc.var_set_string("Name", abbrev);
    sc.fatal_intl(i18n("file type $name unknown"));
    // NOTREACHED
    return "unknown";
}


tree::pointer
shorthand_type(const nstring &abbrev)
{
    nstring name = type_name_by_pattern(abbrev);

    // {}
    tree::pointer tp1 = tree_this::create();

    // type({})
    tree::pointer tp2 = tree_type::create(tp1);

    // name
    rpt_value::pointer vp3 = rpt_value_string::create(name);
    tree::pointer tp3 = tree_constant::create(vp3);

    // type({}) == name
    return tree_eq::create(tp2, tp3);
}
