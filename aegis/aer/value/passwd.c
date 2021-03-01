/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to manipulate passwd values
 */

#include <ac/string.h>
#include <pwd.h>

#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/passwd.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <error.h>
#include <str.h>
#include <trace.h>


static rpt_value_ty *pw_to_struct _((struct passwd *));

static rpt_value_ty *
pw_to_struct(pw)
	struct passwd	*pw;
{
	rpt_value_ty	*result;
	string_ty	*name;
	string_ty	*s;
	rpt_value_ty	*value;
	char		*full_name;
	char		*comma;

	trace(("pw_to_struct()\n{\n"/*}*/));
	result = rpt_value_struct((struct symtab_ty *)0);

	trace(("pw_name\n"));
	name = str_from_c("pw_name");
	s = str_from_c(pw->pw_name);
	value = rpt_value_string(s);
	str_free(s);
	rpt_value_struct__set(result, name, value);
	str_free(name);
	assert(value->reference_count == 2);
	rpt_value_free(value);

	trace(("pw_uid\n"));
	name = str_from_c("pw_uid");
	value = rpt_value_integer((long)pw->pw_uid);
	rpt_value_struct__set(result, name, value);
	str_free(name);
	assert(value->reference_count == 2);
	rpt_value_free(value);

	trace(("pw_gid\n"));
	name = str_from_c("pw_gid");
	value = rpt_value_integer((long)pw->pw_gid);
	rpt_value_struct__set(result, name, value);
	str_free(name);
	assert(value->reference_count == 2);
	rpt_value_free(value);

	trace(("pw_gecos\n"));
	name = str_from_c("pw_gecos");
	if (pw->pw_gecos && pw->pw_gecos[0])
		full_name = pw->pw_gecos;
#ifdef HAVE_pw_comment
	else if (pw->pw_comment && pw->pw_comment[0])
		full_name = pw->pw_comment;
#endif
	else
		full_name = pw->pw_name;

	/*
	 * Some systems add lots of other stuff to the full name field
	 * in the passwd file.  We are only interested in the name.
	 */
	comma = strchr(full_name, ',');
	if (comma)
		s = str_n_from_c(full_name, comma - full_name);
	else
		s = str_from_c(full_name);
	value = rpt_value_string(s);
	str_free(s);
	rpt_value_struct__set(result, name, value);
	str_free(name);
	trace(("pw_comment\n"));
	name = str_from_c("pw_comment");
	rpt_value_struct__set(result, name, value);
	str_free(name);
	assert(value->reference_count == 3);
	rpt_value_free(value);

	trace(("pw_dir\n"));
	name = str_from_c("pw_dir");
	s = str_from_c(pw->pw_dir);
	value = rpt_value_string(s);
	str_free(s);
	rpt_value_struct__set(result, name, value);
	str_free(name);
	assert(value->reference_count == 2);
	rpt_value_free(value);

	trace(("pw_shell\n"));
	name = str_from_c("pw_shell");
	s = str_from_c(pw->pw_shell);
	value = rpt_value_string(s);
	str_free(s);
	rpt_value_struct__set(result, name, value);
	str_free(name);
	assert(value->reference_count == 2);
	rpt_value_free(value);

	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *lookup _((rpt_value_ty *, rpt_value_ty *, int));

static rpt_value_ty *
lookup(lhs, rhs, lvalue)
	rpt_value_ty	*lhs;
	rpt_value_ty	*rhs;
	int		lvalue;
{
	rpt_value_ty	*rhs2;
	rpt_value_ty	*result;
	struct passwd	*pw;

	trace(("value_passwd::lookup()\n{\n"/*}*/));
	rhs2 = rpt_value_arithmetic(rhs);
	if (rhs2->method->type == rpt_value_type_integer)
	{
		int		uid;

		uid = rpt_value_integer_query(rhs2);
		rpt_value_free(rhs2);
		pw = getpwuid(uid);
		if (pw)
			result = pw_to_struct(pw);
		else
			result = rpt_value_error("uid %d unknown", uid);
	}
	else
	{
		rpt_value_free(rhs2);
		rhs2 = rpt_value_stringize(rhs);
		if (rhs2->method->type == rpt_value_type_string)
		{
			string_ty	*name;

			name = rpt_value_string_query(rhs2);
			pw = getpwnam(name->str_text);
			if (pw)
				result = pw_to_struct(pw);
			else
			{
				result =
					rpt_value_error
					(
						"user \"%S\" unknown",
						name
					);
			}
		}
		else
		{
			result =
				rpt_value_error
				(
					"illegal lookup (passwd[%s])",
					rhs->method->name
				);
		}
		rpt_value_free(rhs2);
	}
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *keys _((rpt_value_ty *));

static rpt_value_ty *
keys(vp)
	rpt_value_ty	*vp;
{
	rpt_value_ty	*result;
	struct passwd	*pw;
	string_ty	*s;
	rpt_value_ty	*ep;

	result = rpt_value_list();
	setpwent();
	for (;;)
	{
		pw = getpwent();
		if (!pw)
			break;
		
		s = str_from_c(pw->pw_name);
		ep = rpt_value_string(s);
		str_free(s);
		rpt_value_list_append(result, ep);
		rpt_value_free(ep);
	}
	return result;
}


static rpt_value_ty *count _((rpt_value_ty *));

static rpt_value_ty *
count(vp)
	rpt_value_ty	*vp;
{
	struct passwd	*pw;
	long		n;

	n = 0;
	setpwent();
	for (;;)
	{
		pw = getpwent();
		if (!pw)
			break;
		++n;
	}
	return rpt_value_integer(n);
}


static char *type_of _((rpt_value_ty *));

static char *
type_of(this)
	rpt_value_ty	*this;
{
	return "struct";
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_ty),
	"passwd",
	rpt_value_type_deferred,
	0, /* construct */
	0, /* destruct */
	0, /* arithmetic */
	0, /* stringize */
	0, /* booleanize */
	lookup,
	keys,
	count,
	type_of,
};


rpt_value_ty *
rpt_value_passwd()
{
	static rpt_value_ty *vp;

	if (!vp)
		vp = rpt_value_alloc(&method);
	return rpt_value_copy(vp);
}
