/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate rounds
 */

#include <ac/math.h>

#include <aer/expr.h>
#include <aer/func/round.h>
#include <aer/value/error.h>
#include <aer/value/real.h>
#include <error.h> /* for assert */
#include <str.h>
#include <sub.h>


static int round_verify _((rpt_expr_ty *));

static int
round_verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 1);
}


static rpt_value_ty *round_run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
round_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	double		n;
	rpt_value_ty	*tmp;
	string_ty       *s;
	rpt_value_ty	*result;

	/*
	 * See if it looks like a number.
	 */
	assert(argc == 1);
	tmp = rpt_value_realize(argv[0]);
	if (tmp->method->type != rpt_value_type_real)
	{
		sub_context_ty	*scp;

		rpt_value_free(tmp);
		scp = sub_context_new();
		sub_var_set_charstar(scp, "Function", "round");
		sub_var_set_charstar(scp, "Name", argv[0]->method->name);
		sub_var_set_charstar(scp, "Number", "1");
		s =
			subst_intl
			(
				scp,
      i18n("$function: argument $number: real value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}

	n = rpt_value_real_query(tmp);
	rpt_value_free(tmp);
	n = floor(n + 0.5);
	result = rpt_value_real(n);
	return result;
}


rpt_func_ty rpt_func_round =
{
	"round",
	1, /* optimizable */
	round_verify,
	round_run,
};


static int floor_verify _((rpt_expr_ty *));

static int
floor_verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 1);
}


static rpt_value_ty *floor_run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
floor_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	double		n;
	rpt_value_ty	*tmp;
	string_ty       *s;
	rpt_value_ty	*result;

	/*
	 * See if it looks like a number.
	 */
	assert(argc == 1);
	tmp = rpt_value_realize(argv[0]);
	if (tmp->method->type != rpt_value_type_real)
	{
		sub_context_ty	*scp;

		rpt_value_free(tmp);
		scp = sub_context_new();
		sub_var_set_charstar(scp, "Function", "floor");
		sub_var_set_charstar(scp, "Name", argv[0]->method->name);
		sub_var_set_charstar(scp, "Number", "1");
		s =
			subst_intl
			(
				scp,
      i18n("$function: argument $number: real value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}

	n = rpt_value_real_query(tmp);
	rpt_value_free(tmp);
	n = floor(n);
	result = rpt_value_real(n);
	return result;
}


rpt_func_ty rpt_func_floor =
{
	"floor",
	1, /* optimizable */
	floor_verify,
	floor_run,
};


static int ceil_verify _((rpt_expr_ty *));

static int
ceil_verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 1);
}


static rpt_value_ty *ceil_run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
ceil_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	double		n;
	rpt_value_ty	*tmp;
	string_ty       *s;
	rpt_value_ty	*result;

	/*
	 * See if it looks like a number.
	 */
	assert(argc == 1);
	tmp = rpt_value_realize(argv[0]);
	if (tmp->method->type != rpt_value_type_real)
	{
		sub_context_ty	*scp;

		rpt_value_free(tmp);
		scp = sub_context_new();
		sub_var_set_charstar(scp, "Function", "ceil");
		sub_var_set_charstar(scp, "Name", argv[0]->method->name);
		sub_var_set_charstar(scp, "Number", "1");
		s =
			subst_intl
			(
				scp,
      i18n("$function: argument $number: real value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}

	n = rpt_value_real_query(tmp);
	rpt_value_free(tmp);
	n = ceil(n);
	result = rpt_value_real(n);
	return result;
}


rpt_func_ty rpt_func_ceil =
{
	"ceil",
	1, /* optimizable */
	ceil_verify,
	ceil_run,
};


static int trunc_verify _((rpt_expr_ty *));

static int
trunc_verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 1);
}


static rpt_value_ty *trunc_run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
trunc_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	double		n;
	rpt_value_ty	*tmp;
	string_ty       *s;
	rpt_value_ty	*result;

	/*
	 * See if it looks like a number.
	 */
	assert(argc == 1);
	tmp = rpt_value_realize(argv[0]);
	if (tmp->method->type != rpt_value_type_real)
	{
		sub_context_ty	*scp;

		rpt_value_free(tmp);
		scp = sub_context_new();
		sub_var_set_charstar(scp, "Function", "trunc");
		sub_var_set_charstar(scp, "Name", argv[0]->method->name);
		sub_var_set_charstar(scp, "Number", "1");
		s =
			subst_intl
			(
				scp,
      i18n("$function: argument $number: real value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}

	n = rpt_value_real_query(tmp);
	rpt_value_free(tmp);
	if (n >= 0)
		n = floor(n);
	else
		n = ceil(n);
	result = rpt_value_real(n);
	return result;
}


rpt_func_ty rpt_func_trunc =
{
	"trunc",
	1, /* optimizable */
	trunc_verify,
	trunc_run,
};
