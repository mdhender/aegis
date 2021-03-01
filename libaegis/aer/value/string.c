/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996, 1998, 1999 Peter Miller;
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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate string values
 */

#include <ac/ctype.h>
#include <ac/math.h>
#include <ac/stdlib.h>

#include <aer/value/boolean.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h>
#include <str.h>
#include <trace.h>


typedef struct rpt_value_string_ty rpt_value_string_ty;
struct rpt_value_string_ty
{
	RPT_VALUE
	string_ty	*value;
};


static void destruct _((rpt_value_ty *));

static void
destruct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_string_ty *this;

	this = (rpt_value_string_ty *)vp;
	assert(this->method->type == rpt_value_type_string);
	str_free(this->value);
}


static int is_blank _((char *));

static int
is_blank(s)
	char	*s;
{
	while (isspace(*s))
		++s;
	return !*s;
}


static int is_integer _((char *));

static int
is_integer(s)
	char	*s;
{
	char	*begin;

	while (isspace(*s))
		++s;
	if (*s == '+' || *s == '-')
		++s;
	begin = s;
	while (isdigit(*s))
		++s;
	if (s == begin)
		return 0;
	while (isspace(*s))
		++s;
	return !*s;
}


static int is_real _((char *));

static int
is_real(s)
	char	*s;
{
	char	*begin;

	while (isspace(*s))
		++s;
	if (*s == '+' || *s == '-')
		++s;
	begin = s;
	while (isdigit(*s))
		++s;
	if (*s == '.')
	{
		if (begin == s)
			++begin;
		++s;
		while (isdigit(*s))
			++s;
	}
	if (s == begin)
		return 0;
	if (*s == 'e' || *s == 'E')
	{
		++s;
		if (*s == '+' || *s == '-')
			++s;
		begin = s;
		while (isdigit(*s))
			++s;
		if (s == begin)
			return 0;
	}
	while (isspace(*s))
		++s;
	return !*s;
}


static rpt_value_ty *arithmetic _((rpt_value_ty *));

static rpt_value_ty *
arithmetic(vp)
	rpt_value_ty	*vp;
{
	rpt_value_string_ty *this;
	rpt_value_ty	*result;

	trace(("arithmetic(vp = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_string_ty *)vp;
	assert(this->method->type == rpt_value_type_string);

	if (is_blank(this->value->str_text))
	{
		trace(("blank\n"));
		result = rpt_value_integer(0L);
		trace(("result = %ld;\n", rpt_value_integer_query(result)));
	}
	else if (is_integer(this->value->str_text))
	{
		long		n;

		n = atol(this->value->str_text);
		trace(("integer %ld\n", n));
		result = rpt_value_integer(n);
		trace(("result = %ld;\n", rpt_value_integer_query(result)));
	}
	else if (is_real(this->value->str_text))
	{
		double		n;

		n = atof(this->value->str_text);
		trace(("real %g\n", n));
		result = rpt_value_real(n);
		trace(("result = %g;\n", rpt_value_real_query(result)));
	}
	else
	{
		/*
		 * is can't coerce it to be an arithmetic type,
		 * leave it as a string
		 */
		result = rpt_value_copy(vp);
	}

	trace(("return %08lX;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *booleanize _((rpt_value_ty *));

static rpt_value_ty *
booleanize(vp)
	rpt_value_ty	*vp;
{
	rpt_value_string_ty *this;
	rpt_value_ty	*result;

	trace(("real::booleanize(vp = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_string_ty *)vp;
	assert(this->method->type == rpt_value_type_string);

	if (is_blank(this->value->str_text))
	{
		trace(("blank\n"));
		result = rpt_value_boolean(0);
	}
	else if (is_integer(this->value->str_text))
	{
		long		n;

		n = atol(this->value->str_text);
		trace(("integer %ld\n", n));
		result = rpt_value_boolean(n != 0);
	}
	else if (is_real(this->value->str_text))
	{
		double		n;

		n = atof(this->value->str_text);
		trace(("real %g\n", n));
		result = rpt_value_boolean(n != 0);
	}
	else
	{
		/*
		 * is can't coerce it to be boolean,
		 * leave it as a string
		 */
		result = rpt_value_copy(vp);
	}

	trace(("return %08lX;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_string_ty),
	"string",
	rpt_value_type_string,
	0, /* construct */
	destruct,
	arithmetic,
	0, /* stringize */
	booleanize,
	0, /* lookup */
	0, /* keys */
	0, /* count */
	0, /* type_of */
	0, /* undefer */
};


rpt_value_ty *
rpt_value_string(s)
	string_ty	*s;
{
	rpt_value_string_ty *this;

	this = (rpt_value_string_ty *)rpt_value_alloc(&method);
	this->value = str_copy(s);
	return (rpt_value_ty *)this;
}


string_ty *
rpt_value_string_query(vp)
	rpt_value_ty	*vp;
{
	rpt_value_string_ty *this;

	this = (rpt_value_string_ty *)vp;
	assert(this->method->type == rpt_value_type_string);
	return this->value;
}
