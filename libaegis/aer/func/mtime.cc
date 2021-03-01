//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to implement the builtin mtime function
//

#include <ac/errno.h>
#include <ac/stddef.h>
#include <ac/stdlib.h>
#include <ac/sys/types.h>
#include <sys/stat.h>

#include <aer/expr.h>
#include <aer/func/mtime.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <aer/value/time.h>
#include <error.h>
#include <glue.h>
#include <os.h>
#include <sub.h>


static int
verify(rpt_expr_ty *ep)
{
	return (ep->nchild == 1);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
	rpt_value_ty	*tmp;
	string_ty       *s;
	rpt_value_ty	*result;
	struct stat	st;
	int		err;

	assert(argc == 1);

	//
	// Coerce the argument to a string.
	// It is an error if it can't be.
	//
	tmp = rpt_value_stringize(argv[0]);
	if (tmp->method->type != rpt_value_type_string)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(tmp);
		sub_var_set_charstar(scp, "Function", "mtime");
		sub_var_set_long(scp, "Number", 1);
		sub_var_set_charstar(scp, "Name", argv[0]->method->name);
		s =
			subst_intl
			(
				scp,
    i18n("$function: argument $number: string value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}

	//
	// stat the file
	//
	s = rpt_value_string_query(tmp);
	rpt_value_free(tmp);
	os_become_orig();
	err = 0;
	if (glue_lstat(s->str_text, &st) < 0)
		err = errno;
        os_become_undo();
	if (err != 0)
	{
		sub_context_ty	*scp;
		string_ty	*errstr;

		//
		// If there was a problem, build an error result.
		//
		scp = sub_context_new();
		sub_errno_setx(scp, err);
		sub_var_set_string(scp, "File_Name", s);
		errstr = subst_intl(scp, "stat $filename: $errno");
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, errstr);
		str_free(errstr);
	}
	else
	{
		//
		// Build a time from the statbuf.
		//
		result = rpt_value_time(st.st_mtime);
	}
	str_free(s);

	//
	// all done
	//
	return result;
}


rpt_func_ty rpt_func_mtime =
{
	"mtime",
	0, // optimizable
	verify,
	run,
};
