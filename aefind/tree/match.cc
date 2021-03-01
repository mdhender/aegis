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

#include <common/gmatch.h>
#include <common/str.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>

#include <aefind/tree/diadic.h>
#include <aefind/tree/match.h>


tree_match::~tree_match()
{
}


tree_match::tree_match(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_match::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_match(a1, a2));
}


rpt_value::pointer
tree_match::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
    rpt_value::pointer v1s = rpt_value::stringize(v1);
    rpt_value_string *v1sp = dynamic_cast<rpt_value_string *>(v1s.get());

    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
    rpt_value::pointer v2s = rpt_value::stringize(v2);
    rpt_value_string *v2sp = dynamic_cast<rpt_value_string *>(v2s.get());

    if (!v1sp || !v2sp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name1", v1->name());
	sc.var_set_charstar("Name2", v2->name());
	nstring s(sc.subst_intl(i18n("illegal match ($name1 ~ $name2)")));
	return rpt_value_error::create(s);
    }
    return
        rpt_value_boolean::create
        (
            gmatch(v1sp->query().c_str(), v2sp->query().c_str())
        );
}


tree::pointer
tree_match::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_match::name()
    const
{
    return "~~";
}
